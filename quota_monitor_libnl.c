#include <errno.h>
#include <linux/quota.h>

#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>

#include "common.h"

static struct nla_policy quota_nl_warn_cmd_policy[QUOTA_NL_A_MAX+1] = {
  [QUOTA_NL_A_QTYPE] = { .type = NLA_U32 },
  [QUOTA_NL_A_EXCESS_ID] = { .type = NLA_U64 },
  [QUOTA_NL_A_WARNING] = { .type = NLA_U32 },
  [QUOTA_NL_A_DEV_MAJOR] = { .type = NLA_U32 },
  [QUOTA_NL_A_DEV_MINOR] = { .type = NLA_U32 },
  [QUOTA_NL_A_CAUSED_ID] = { .type = NLA_U64 },
};

static int
quota_nl_parser (struct nl_msg *msg, void *arg)
{
  struct nlmsghdr *nlh = nlmsg_hdr(msg);
  struct genlmsghdr *ghdr;
  struct nlattr *attrs[QUOTA_NL_A_MAX+1];
  int ret;

  (void)arg;

  if (!genlmsg_valid_hdr(nlh, 0))
    return 0;
  ghdr = nlmsg_data(nlh);

  if (ghdr->cmd != QUOTA_NL_C_WARNING)
    {
      fprintf (stderr, "Unknown cmd %d\n", ghdr->cmd);
      return 0;
    }

  ret = genlmsg_parse (nlh, 0, attrs, QUOTA_NL_A_MAX, quota_nl_warn_cmd_policy);
  if (ret < 0)
    {
      fprintf (stderr, "Error parsing netlink message.\n");
      return ret;
    }

  if (!attrs[QUOTA_NL_A_QTYPE] || !attrs[QUOTA_NL_A_EXCESS_ID] ||
      !attrs[QUOTA_NL_A_WARNING] || !attrs[QUOTA_NL_A_DEV_MAJOR] ||
      !attrs[QUOTA_NL_A_DEV_MINOR] || !attrs[QUOTA_NL_A_CAUSED_ID])
    {
      fprintf (stderr, "Mandatory attributes has not come.\n");
      return -EINVAL;
    }

  u_int32_t qtype     = nla_get_u32(attrs[QUOTA_NL_A_QTYPE]);
  u_int64_t excess_id = nla_get_u64(attrs[QUOTA_NL_A_EXCESS_ID]);
  u_int32_t warning   = nla_get_u32(attrs[QUOTA_NL_A_WARNING]);
  u_int32_t dev_major = nla_get_u32(attrs[QUOTA_NL_A_DEV_MAJOR]);
  u_int32_t dev_minor = nla_get_u32(attrs[QUOTA_NL_A_DEV_MINOR]);
  u_int64_t caused_id = nla_get_u64(attrs[QUOTA_NL_A_CAUSED_ID]);

  printf ("--------------------------------\n");
  printf ("QUOTA_NL_A_QTYPE = %u (%s)\n", qtype, qtype_to_str(qtype));
  printf ("QUOTA_NL_A_EXCESS_ID = %lu\n", excess_id);
  printf ("QUOTA_NL_A_WARNING = %u (%s)\n", warning, warning_to_str(warning));
  printf ("QUOTA_NL_A_DEV_MAJOR = %u\n", dev_major);
  printf ("QUOTA_NL_A_DEV_MINOR = %u\n", dev_minor);
  printf ("QUOTA_NL_A_CAUSED_ID = %lu\n", caused_id);

  return 0;
}

static struct nl_sock *
init_netlink (void)
{
  struct nl_sock *sock;
  int ret, mc_family;

  sock = nl_socket_alloc();
  if (!sock)
    {
      fprintf (stderr, "Cannot allocate netlink socket!\n");
      exit (EXIT_FAILURE);
    }
  nl_socket_disable_seq_check(sock);

  ret = nl_socket_modify_cb(sock, NL_CB_VALID, NL_CB_CUSTOM,
                            quota_nl_parser, NULL);
  if (ret < 0)
    {
      fprintf (stderr, "Cannot register callback for"
               " netlink messages: %s\n", strerror(-ret));
      exit (EXIT_FAILURE);
    }

  ret = genl_connect(sock);
  if (ret < 0)
    {
      fprintf (stderr, "Cannot connect to netlink socket: %s\n", strerror(-ret));
      exit (EXIT_FAILURE);
    }

  mc_family = genl_ctrl_resolve_grp(sock, "VFS_DQUOT", "events");
  if (mc_family < 0)
    {
      /*
       * Using family id for multicasting is wrong but I messed up
       * kernel netlink interface by using family id as a multicast
       * group id in kernel so we have to carry this code to keep
       * compatibility with older kernels.
       */
      mc_family = genl_ctrl_resolve(sock, "VFS_DQUOT");
      if (mc_family < 0)
        {
          fprintf (stderr, "Cannot resolve quota netlink name: %s\n",
                   strerror(-mc_family));
          exit (EXIT_FAILURE);
        }
    }

  ret = nl_socket_add_membership(sock, mc_family);
  if (ret < 0)
    {
      fprintf (stderr, "Cannot join quota multicast group: %s\n", strerror(-ret));
      exit (EXIT_FAILURE);
    }

  return sock;
}

int
main (void)
{
  struct nl_sock *nsock = init_netlink ();

  do
    {
      int ret = nl_recvmsgs_default(nsock);
      if (ret < 0)
        {
          fprintf (stderr, "Failed to read or parse quota netlink"
                   " message: %s\n", strerror(-ret));
          exit (EXIT_FAILURE);
        }
    }
  while (1);

  return 0;
}
