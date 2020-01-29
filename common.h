#ifndef INCLUDED_COMMON_H
#define INCLUDED_COMMON_H

static inline char*
nla_type_to_str (__u16 nla_type)
{
  switch (nla_type)
    {
    case QUOTA_NL_A_QTYPE:     return "QUOTA_NL_A_QTYPE";
    case QUOTA_NL_A_EXCESS_ID: return "QUOTA_NL_A_EXCESS_ID";
    case QUOTA_NL_A_WARNING:   return "QUOTA_NL_A_WARNING";
    case QUOTA_NL_A_DEV_MAJOR: return "QUOTA_NL_A_DEV_MAJOR";
    case QUOTA_NL_A_DEV_MINOR: return "QUOTA_NL_A_DEV_MINOR";
    case QUOTA_NL_A_CAUSED_ID: return "QUOTA_NL_A_CAUSED_ID";
    default:                   return "unknwon";
    }
}

static inline char*
qtype_to_str (u_int32_t qtype)
{
  switch (qtype)
    {
    case USRQUOTA: return "USRQUOTA";
    case GRPQUOTA: return "GRPQUOTA";
    case PRJQUOTA: return "PRJQUOTA";
    default:       return "unknwon";
    }
}

static inline char*
warning_to_str (u_int32_t warning)
{
  switch (warning)
    {
    case QUOTA_NL_NOWARN:        return "QUOTA_NL_NOWARN";
    case QUOTA_NL_IHARDWARN:     return "QUOTA_NL_IHARDWARN";
    case QUOTA_NL_ISOFTLONGWARN: return "QUOTA_NL_ISOFTLONGWARN";
    case QUOTA_NL_ISOFTWARN:     return "QUOTA_NL_ISOFTWARN";
    case QUOTA_NL_BHARDWARN:     return "QUOTA_NL_BHARDWARN";
    case QUOTA_NL_BSOFTLONGWARN: return "QUOTA_NL_BSOFTLONGWARN";
    case QUOTA_NL_BSOFTWARN:     return "QUOTA_NL_BSOFTWARN";
    case QUOTA_NL_IHARDBELOW:    return "QUOTA_NL_IHARDBELOW";
    case QUOTA_NL_ISOFTBELOW:    return "QUOTA_NL_ISOFTBELOW";
    case QUOTA_NL_BHARDBELOW:    return "QUOTA_NL_BHARDBELOW";
    case QUOTA_NL_BSOFTBELOW:    return "QUOTA_NL_BSOFTBELOW";
    default:                     return "unknwon";
    }
}

#endif /* INCLUDED_COMMON_H */
