// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#include "ProxyMock.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "ExtUtils.h"
#include "TraceUtils.h"
#include "checks.h"
#include "comm.h"
#include "nccl.h"

/*
=== BEGIN_NCCL_CVAR_INFO_BLOCK ===
 - name        : NCCL_PROXYMOCK_NET_SEND_FAILURE
   type        : stringlist
   default     :
   description : |-
     Backdoor to mock a send failure in proxy thread following the format of
     "<opCount>,<rank>,<remoteRank>,<step>,<num_match>,<delay_sec>".
     Set any of <opCount>,<rank>,<remoteRank>,<step> to -1 to match any.
     Set <delay_sec> to 0 to always skip this send as a network failure,
     otherwise delay it in seconds to mock slow network.
     Set <num_match> to control how many times to match the send failure.
     Example 1: "10,1,0,5,1,30" - mock a slow send by delaying 30 seconds
     for the send operation matching [opCount 10, channel 1, rank 0, remote
     rank 1, starting from step 5].
     Example 2: "10,1,-1,-1,2,0" - mock 2 send failures matching [opCount 10,
     channel 1, rank 0, *any* remote rank, any step].

=== END_NCCL_CVAR_INFO_BLOCK ===
*/

static std::vector<std::string> netSendFailureKeys =
    {"opCount", "rank", "remoteRank", "step", "numMatch", "delaySec"};

std::string ProxyMockNetSendFailure::serialize(bool quoted) {
  std::unordered_map<std::string, std::string> map;
  map["opCount"] = std::to_string(opCount_);
  map["rank"] = std::to_string(rank_);
  map["remoteRank"] = std::to_string(remoteRank_);
  map["step"] = std::to_string(step_);
  map["numMatch"] = std::to_string(numMatch_);
  map["delaySec"] = std::to_string(delaySec_);

  return serializeMap(netSendFailureKeys, map, quoted);
}

ProxyMockNetSendFailure::ProxyMockNetSendFailure() {
  initialize();
}

void ProxyMockNetSendFailure::initialize() {
  // no lock here since we assume direct call would be only in tests and only
  // after communicator destroy

  // Reset all fields if set
  enabled_ = false;
  mockStartMap_.clear();
  numMatched_ = 0;

  // Enable mock only when all required fields are set
  if (NCCL_PROXYMOCK_NET_SEND_FAILURE.size() == netSendFailureKeys.size()) {
    opCount_ = std::stoi(NCCL_PROXYMOCK_NET_SEND_FAILURE[0]);
    rank_ = std::stoi(NCCL_PROXYMOCK_NET_SEND_FAILURE[1]);
    remoteRank_ = std::stoi(NCCL_PROXYMOCK_NET_SEND_FAILURE[2]);
    step_ = std::stoi(NCCL_PROXYMOCK_NET_SEND_FAILURE[3]);
    numMatch_ = std::stoi(NCCL_PROXYMOCK_NET_SEND_FAILURE[4]);
    delaySec_ = std::stoi(NCCL_PROXYMOCK_NET_SEND_FAILURE[5]);

    enabled_ = true;
    std::string configStr = serialize();
    INFO(
        NCCL_ENV | NCCL_INIT,
        "PROXYMOCK: enabled NetSendFailure with config %s",
        configStr.c_str());
  } else if (!NCCL_PROXYMOCK_NET_SEND_FAILURE.empty()) {
    WARN(
        "PROXYMOCK: invalid value of NCCL_PROXYMOCK_NET_SEND_FAILURE. Valid format: %s",
        vecToStr(netSendFailureKeys, ",").c_str());
  }
}

bool ProxyMockNetSendFailure::mockImpl(
    struct ncclProxySubArgs* sub,
    int currStep,
    void** request) {
  std::lock_guard<std::mutex> lock(mutex_);
  bool mocked = false;

  if ((opCount_ == PROXYMOCK_MATCH_ANY ||
       sub->traceArgs.collInfo.opCount == opCount_) &&
      (rank_ == PROXYMOCK_MATCH_ANY || sub->traceArgs.rank == rank_) &&
      (remoteRank_ == PROXYMOCK_MATCH_ANY ||
       sub->traceArgs.remoteRank == remoteRank_) &&
      (step_ == PROXYMOCK_MATCH_ANY || currStep >= step_)) {
    std::string mockConfigStr = this->serialize();

    // Only warn the first time, because proxy thread will hang here and repeat
    // the mock
    auto cond = NetSendFailureCond(
        sub->traceArgs.collInfo.commHash,
        sub->traceArgs.collInfo.opCount,
        sub->traceArgs.proxyOpId,
        sub->traceArgs.rank,
        sub->traceArgs.remoteRank,
        currStep);
    auto condStr = cond.toString();

    // A new mock match if the SEND is not matched yet and within numMatch_
    // limit
    if (mockStartMap_.count(cond) == 0 && numMatched_ < numMatch_) {
      WARN(
          "PROXYMOCK: Mocked send failure (matched %d times), skiped SEND at %s with config %s",
          numMatched_,
          condStr.c_str(),
          mockConfigStr.c_str());
      // Record hanging start time
      mockStartMap_[cond] = std::chrono::high_resolution_clock::now();
      mocked = true;

      // Use seperate counter to track how many times it is mocked, since a mock
      // can be erased from mockStartMap_ if delaySec is set
      numMatched_++;
    } else if (mockStartMap_.count(cond)) {
      mocked = true;
      // If being mocked, check if it is time to continue or stay hanging
      if (delaySec_ > 0) {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            now - mockStartMap_[cond]);
        if (duration.count() > delaySec_) {
          WARN(
              "PROXYMOCK: Mocked send failure at %s has been hanging for more than %d seconds, continue execution",
              condStr.c_str(),
              delaySec_);

          mockStartMap_.erase(cond);
          mocked = false;
        }
      } else {
        // always stay hanging
        mocked = true;
      }
    }
  }

  if (mocked) {
    // Set request to NULL so that net.cc would not check CQE associated with
    // it and assumes post_send is not yet called
    *request = NULL;
  }

  return mocked;
}

ProxyMockNetSendFailure& ProxyMockNetSendFailure::getInstance() {
  static ProxyMockNetSendFailure instance_;
  return instance_;
}

bool ProxyMockNetSendFailure::mock(
    struct ncclProxySubArgs* sub,
    int step,
    void** request) {
  auto& instance = getInstance();
  if (instance.enabled_) {
    return instance.mockImpl(sub, step, request);
  } else {
    return false;
  }
}
