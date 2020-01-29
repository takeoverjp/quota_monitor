#include <asm/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>
#include <linux/quota.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

#define VERBOSE 0

static void
dump_nlmsghdr (struct nlmsghdr *nh)
{
  printf ("NLMSG_HDRLEN = %u\n", NLMSG_HDRLEN);
  printf ("  nh->nlmsg_len = %u\n", nh->nlmsg_len);
  printf ("  nh->nlmsg_type = %u (%s)\n", nh->nlmsg_type,
          nh->nlmsg_type == GENL_ID_VFS_DQUOT ? "GENL_ID_VFS_DQUOT" : "unknown");
  printf ("  nh->nlmsg_flags = 0x%x\n", nh->nlmsg_flags);
  printf ("  nh->nlmsg_seq = %u\n", nh->nlmsg_seq);
  printf ("  nh->nlmsg_pid = %u\n", nh->nlmsg_pid);
}

static void
dump_genlmsghdr (struct genlmsghdr *gh)
{
  printf ("GENL_HDRLEN = %lu\n", GENL_HDRLEN);
  printf ("  gh->cmd = %d (%s)\n", gh->cmd,
          gh->cmd == QUOTA_NL_C_WARNING ? "QUOTA_NL_C_WARNING" : "unkown");
  printf ("  gh->version = %d\n", gh->version);
}

static void
dump_nlattr (struct nlattr *attr)
{
  printf ("NLA_HDRLEN = %d\n", NLA_HDRLEN);
  printf ("  attr->nla_len = %d\n", attr->nla_len);
  printf ("  attr->nla_type = %d (%s)\n", attr->nla_type, nla_type_to_str (attr->nla_type));
  
}

int
main(void)
{
  struct sockaddr_nl src_addr;

  int sock = socket (PF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock < 0)
    {
      perror ("socket");
      exit (EXIT_FAILURE);
    }

  memset (&src_addr, 0, sizeof(src_addr));
  src_addr.nl_family = AF_NETLINK;

  int ret = bind (sock, (struct sockaddr*)&src_addr, sizeof(src_addr));
  if (ret < 0)
    {
      perror ("bind");
      exit (EXIT_FAILURE);
    }

  /* $ genl-ctrl-list | grep VFS_DQUOT */
  int group = GENL_ID_VFS_DQUOT;
  ret = setsockopt (sock, SOL_NETLINK, NETLINK_ADD_MEMBERSHIP,
                    &group, sizeof(group));
  if (ret < 0)
    {
      perror ("setsockopt");
      exit (EXIT_FAILURE);
    }

  struct sockaddr_nl nla;
  char buf[8192];
  struct iovec iov = {buf, sizeof(buf)};
  struct msghdr msg = {
    .msg_name = (void *) &nla,
    .msg_namelen = sizeof(nla),
    .msg_iov = &iov,
    .msg_iovlen = 1,
  };

  do
    {
      int len = recvmsg (sock, &msg, 0);
      for (struct nlmsghdr *nh = (struct nlmsghdr *)buf;
           NLMSG_OK(nh, len);
           nh = NLMSG_NEXT(nh, len))
        {
          if (nh->nlmsg_type == NLMSG_DONE)
            break;
          
          if (nh->nlmsg_type == NLMSG_ERROR)
            {
              fprintf (stderr, "NLMSG_ERROR\n");
              exit (EXIT_FAILURE);
            }
          
          struct genlmsghdr *gh = NLMSG_DATA(nh);
          char * genlmsg_data = (char *) gh + GENL_HDRLEN;
          struct nlattr * attr[QUOTA_NL_A_MAX+1];
          attr[1] = (struct nlattr *)genlmsg_data;
          for (int i = 1; i < QUOTA_NL_A_MAX; i++)
            attr[i+1] = (struct nlattr *)((char *)attr[i] + attr[i]->nla_len);


          printf ("--------------------------------\n");

          if (VERBOSE)
            {
              dump_nlmsghdr (nh);
              dump_genlmsghdr (gh);
              for (int i = 1; i < QUOTA_NL_A_MAX; i++)
                dump_nlattr (attr[i]);
            }

          u_int32_t qtype     = *(u_int32_t*)((char *)attr[QUOTA_NL_A_QTYPE]     + NLA_HDRLEN);
          u_int64_t excess_id = *(u_int64_t*)((char *)attr[QUOTA_NL_A_EXCESS_ID] + NLA_HDRLEN);
          u_int32_t warning   = *(u_int32_t*)((char *)attr[QUOTA_NL_A_WARNING]   + NLA_HDRLEN);
          u_int32_t dev_major = *(u_int32_t*)((char *)attr[QUOTA_NL_A_DEV_MAJOR] + NLA_HDRLEN);
          u_int32_t dev_minor = *(u_int32_t*)((char *)attr[QUOTA_NL_A_DEV_MINOR] + NLA_HDRLEN);
          u_int64_t caused_id = *(u_int64_t*)((char *)attr[QUOTA_NL_A_CAUSED_ID] + NLA_HDRLEN);

          printf ("QUOTA_NL_A_QTYPE = %u (%s)\n", qtype, qtype_to_str(qtype));
          printf ("QUOTA_NL_A_EXCESS_ID = %lu\n", excess_id);
          printf ("QUOTA_NL_A_WARNING = %u (%s)\n", warning, warning_to_str(warning));
          printf ("QUOTA_NL_A_DEV_MAJOR = %u\n", dev_major);
          printf ("QUOTA_NL_A_DEV_MINOR = %u\n", dev_minor);
          printf ("QUOTA_NL_A_CAUSED_ID = %lu\n", caused_id);
        }
    }
  while (1);
  
  close (sock);
}
