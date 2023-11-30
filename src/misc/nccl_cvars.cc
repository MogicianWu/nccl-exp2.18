// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#include <sys/syscall.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <unordered_set>
#include <vector>
#include <cstring>
#include "nccl_cvars.h"
#include "debug.h"
#include "checks.h"

/**
* Cvar template source file.
* It initializes control variables for NCCL at environment initialization time.
* Partial contents are auto-generated by ./maint/extractcvars.py
* See ## CONTENTS ## in nccl_cvars.cc.in
*/

#define CVAR_WARN_UNKNOWN_VALUE(name, value)               \
  do {                                                     \
    WARN("Unknown value %s for env %s", name, value);      \
  } while (0)

// trim from start (in place)
static inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
    return !std::isspace(ch);
  }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
    return !std::isspace(ch);
  }).base(), s.end());
}

static std::vector<std::string> tokenizer(std::string str) {
  std::string delim = ",";
  std::vector<std::string> tokens;

  while (auto pos = str.find(",")) {
    std::string newstr = str.substr(0, pos);
    ltrim(newstr);
    rtrim(newstr);
    // Skip empty string
    if(!newstr.empty()) {
      if(std::find(tokens.begin(), tokens.end(), newstr) != tokens.end()) {
        // WARN("Duplicate token %s found in the value of %s", newstr.c_str(), str_);
      }
      tokens.push_back(newstr);
    }
    str.erase(0, pos + delim.length());
    if (pos == std::string::npos) {
      break;
    }
  }
  return tokens;
}

static bool env2bool(const char *str_, const char *def) {
  std::string str(getenv(str_) ? getenv(str_) : def);
  std::transform(str.cbegin(), str.cend(), str.begin(), [](unsigned char c) { return std::tolower(c); });
  if (str == "y") return true;
  else if (str == "n") return false;
  else if (str == "yes") return true;
  else if (str == "no") return false;
  else if (str == "t") return true;
  else if (str == "f") return false;
  else if (str == "true") return true;
  else if (str == "false") return false;
  else if (str == "1") return true;
  else if (str == "0") return false;
  // else CVAR_WARN_UNKNOWN_VALUE(str_, str.c_str());
  return true;
}

static int env2int(const char *str, const char *def) {
  return getenv(str) ? atoi(getenv(str)) : atoi(def);
}

static std::string env2str(const char *str, const char *def_) {
  const char *def = def_ ? def_ : "";
  std::string str_s = getenv(str) ? std::string(getenv(str)) : std::string(def);
  ltrim(str_s);
  rtrim(str_s);
  return str_s;
}

static std::vector<std::string> env2strlist(const char* str, const char* def_) {
  const char* def = def_ ? def_ : "";
  std::string str_s(getenv(str) ? getenv(str) : def);
  return tokenizer(str_s);
}

std::tuple<std::string, std::vector<std::string>> env2prefixedStrlist(
    const char* str,
    const char* def_,
    std::vector<std::string> prefixes) {
  const char* def = def_ ? def_ : "";
  std::string str_s(getenv(str) ? getenv(str) : def);

  // search if any prefix is specified
  for (auto prefix : prefixes) {
    if (!str_s.compare(0, prefix.size(), prefix)) {
      // if prefix is found, convert the remaining string to stringList
      std::string slist_s = str_s.substr(prefix.size());
      return std::make_tuple(prefix, tokenizer(slist_s));
    }
  }
  // if no prefix is found, convert entire string to stringList
  return std::make_tuple("", tokenizer(str_s));
}

// Automatically generated by ./maint/extractcvars.py --- START
// DO NOT EDIT!!!
bool NCCL_DDA_ALLREDUCE_LARGE_MESSAGE_HCM;

int NCCL_DDA_ALLREDUCE_TMPBUFF_SIZE;

int NCCL_DDA_MAX_RANKS;

enum NCCL_ALLREDUCE_ALGO NCCL_ALLREDUCE_ALGO;

enum NCCL_ALLREDUCE_ALGO2 NCCL_ALLREDUCE_ALGO2;

int NCCL_ALLGATHER_DIRECT_CUTOFF;

int NCCL_DDA_ALLREDUCE_MAX_BLOCKS;

int NCCL_DDA_ALLREDUCE_TREE_THRESHOLD_NVS;

int NCCL_DDA_ALLREDUCE_TREE_THRESHOLD_HCM;

int NCCL_ALLREDUCE_SPARSE_BLOCK_NUM_THREAD_BLOCKS;

int NCCL_ALLREDUCE_SPARSE_BLOCK_THREAD_BLOCK_SIZE;

bool NCCL_DDA_FORCE_P2P_ACCESS;

std::string NCCL_IB_HCA_PREFIX;
std::vector<std::string> NCCL_IB_HCA;

int NCCL_CTRAN_IB_MAX_QPS;

int NCCL_CTRAN_IB_QP_SCALING_THRESHOLD;

// Automatically generated by ./maint/extractcvars.py --- END


extern char **environ;
static std::unordered_set<std::string> env;

// Automatically generated by ./maint/extractcvars.py --- START
// DO NOT EDIT!!!
void initEnvSet() {
  env.insert("NCCL_DDA_ALLREDUCE_LARGE_MESSAGE_HCM");
  env.insert("NCCL_DDA_ALLREDUCE_TMPBUFF_SIZE");
  env.insert("NCCL_DDA_MAX_RANKS");
  env.insert("NCCL_ALLREDUCE_ALGO");
  env.insert("NCCL_ALLREDUCE_ALGO2");
  env.insert("NCCL_ALLGATHER_DIRECT_CUTOFF");
  env.insert("NCCL_DDA_ALLREDUCE_MAX_BLOCKS");
  env.insert("NCCL_DDA_ALLREDUCE_TREE_THRESHOLD_NVS");
  env.insert("NCCL_DDA_ALLREDUCE_TREE_THRESHOLD_HCM");
  env.insert("NCCL_ALLREDUCE_SPARSE_BLOCK_NUM_THREAD_BLOCKS");
  env.insert("NCCL_ALLREDUCE_SPARSE_BLOCK_THREAD_BLOCK_SIZE");
  env.insert("NCCL_DDA_FORCE_P2P_ACCESS");
  env.insert("NCCL_IB_HCA");
  env.insert("NCCL_CTRAN_IB_MAX_QPS");
  env.insert("NCCL_CTRAN_IB_QP_SCALING_THRESHOLD");
  env.insert("NCCL_ALGO");
  env.insert("NCCL_COLLNET_ENABLE");
  env.insert("NCCL_COLLTRACE_LOCAL_SUBDIR");
  env.insert("NCCL_COMM_ID");
  env.insert("NCCL_CUDA_PATH");
  env.insert("NCCL_CROSS_NIC");
  env.insert("NCCL_DEBUG");
  env.insert("NCCL_DEBUG_FILE");
  env.insert("NCCL_DEBUG_SUBSYS");
  env.insert("NCCL_GRAPH_DUMP_FILE");
  env.insert("NCCL_GRAPH_FILE");
  env.insert("NCCL_HOSTID");
  env.insert("NCCL_IB_DISABLE");
  env.insert("NCCL_IB_GID_INDEX");
  env.insert("NCCL_IB_TC");
  env.insert("NCCL_IB_TIMEOUT");
  env.insert("NCCL_IB_QPS_PER_CONNECTION");
  env.insert("NCCL_LAUNCH_MODE");
  env.insert("NCCL_NET");
  env.insert("NCCL_NET_PLUGIN");
  env.insert("NCCL_NET_SHARED_COMMS");
  env.insert("NCCL_NSOCKS_PERTHREAD");
  env.insert("NCCL_PROTO");
  env.insert("NCCL_PROXY_PROFILE");
  env.insert("NCCL_PXN_DISABLE");
  env.insert("NCCL_P2P_LEVEL");
  env.insert("NCCL_SHM_DISABLE");
  env.insert("NCCL_SOCKET_FAMILY");
  env.insert("NCCL_SOCKET_IFNAME");
  env.insert("NCCL_SOCKET_NTHREADS");
  env.insert("NCCL_THREAD_THRESHOLDS");
  env.insert("NCCL_TOPO_DUMP_FILE");
  env.insert("NCCL_TOPO_FILE");
  env.insert("NCCL_TUNER_PLUGIN");
}
// Automatically generated by ./maint/extractcvars.py --- END


// Automatically generated by ./maint/extractcvars.py --- START
// DO NOT EDIT!!!
void readCvarEnv() {
  NCCL_DDA_ALLREDUCE_LARGE_MESSAGE_HCM = env2bool("NCCL_DDA_ALLREDUCE_LARGE_MESSAGE_HCM", "False");

  NCCL_DDA_ALLREDUCE_TMPBUFF_SIZE = env2int("NCCL_DDA_ALLREDUCE_TMPBUFF_SIZE", "33554432");

  NCCL_DDA_MAX_RANKS = env2int("NCCL_DDA_MAX_RANKS", "16");

  if (getenv("NCCL_ALLREDUCE_ALGO") == nullptr) {
    NCCL_ALLREDUCE_ALGO = NCCL_ALLREDUCE_ALGO::orig;
  } else {
    std::string str(getenv("NCCL_ALLREDUCE_ALGO"));
    if (str == std::string("orig")) {
      NCCL_ALLREDUCE_ALGO = NCCL_ALLREDUCE_ALGO::orig;
    } else if (str == std::string("dda")) {
      NCCL_ALLREDUCE_ALGO = NCCL_ALLREDUCE_ALGO::dda;
    } else {
      CVAR_WARN_UNKNOWN_VALUE("NCCL_ALLREDUCE_ALGO", str.c_str());
    }
  }

  if (getenv("NCCL_ALLREDUCE_ALGO2") == nullptr) {
    NCCL_ALLREDUCE_ALGO2 = NCCL_ALLREDUCE_ALGO2::orig;
  } else {
    std::string str(getenv("NCCL_ALLREDUCE_ALGO2"));
    if (str == std::string("orig")) {
      NCCL_ALLREDUCE_ALGO2 = NCCL_ALLREDUCE_ALGO2::orig;
    } else if (str == std::string("dda")) {
      NCCL_ALLREDUCE_ALGO2 = NCCL_ALLREDUCE_ALGO2::dda;
    } else {
      CVAR_WARN_UNKNOWN_VALUE("NCCL_ALLREDUCE_ALGO2", str.c_str());
    }
  }

  NCCL_ALLGATHER_DIRECT_CUTOFF = env2int("NCCL_ALLGATHER_DIRECT_CUTOFF", "524288");

  NCCL_DDA_ALLREDUCE_MAX_BLOCKS = env2int("NCCL_DDA_ALLREDUCE_MAX_BLOCKS", "1");

  NCCL_DDA_ALLREDUCE_TREE_THRESHOLD_NVS = env2int("NCCL_DDA_ALLREDUCE_TREE_THRESHOLD_NVS", "262144");

  NCCL_DDA_ALLREDUCE_TREE_THRESHOLD_HCM = env2int("NCCL_DDA_ALLREDUCE_TREE_THRESHOLD_HCM", "65536");

  NCCL_ALLREDUCE_SPARSE_BLOCK_NUM_THREAD_BLOCKS = env2int("NCCL_ALLREDUCE_SPARSE_BLOCK_NUM_THREAD_BLOCKS", "-1");

  NCCL_ALLREDUCE_SPARSE_BLOCK_THREAD_BLOCK_SIZE = env2int("NCCL_ALLREDUCE_SPARSE_BLOCK_THREAD_BLOCK_SIZE", "-1");

  NCCL_DDA_FORCE_P2P_ACCESS = env2bool("NCCL_DDA_FORCE_P2P_ACCESS", "False");

  std::vector<std::string> NCCL_IB_HCA_allPrefixes{"^", "="};
  NCCL_IB_HCA.clear();
  std::tie(NCCL_IB_HCA_PREFIX, NCCL_IB_HCA) = env2prefixedStrlist("NCCL_IB_HCA", "", NCCL_IB_HCA_allPrefixes);

  NCCL_CTRAN_IB_MAX_QPS = env2int("NCCL_CTRAN_IB_MAX_QPS", "1");

  NCCL_CTRAN_IB_QP_SCALING_THRESHOLD = env2int("NCCL_CTRAN_IB_QP_SCALING_THRESHOLD", "1048576");

}
// Automatically generated by ./maint/extractcvars.py --- END


void ncclCvarInit() {
  initEnvSet();

  // Check if any NCCL_ env var is not in allow list
  char **s = environ;
  for (; *s; s++) {
    if (!strncmp(*s, "NCCL_", strlen("NCCL_"))) {
      std::string str(*s);
      str = str.substr(0, str.find("="));
      if (env.find(str) == env.end()) {
        // CVAR_WARN("Unknown env %s in the NCCL namespace\n", str.c_str());
      }
    }
  }

  readCvarEnv();
}
