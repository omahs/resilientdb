/*
 * Copyright (c) 2019-2022 ExpoLab, UC Davis
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#pragma once

#include "chain/storage/txn_memory_db.h"
#include "common/crypto/signature_verifier.h"
#include "platform/config/resdb_config.h"
#include "platform/consensus/checkpoint/checkpoint.h"
#include "platform/networkstrate/replica_communicator.h"
#include "platform/networkstrate/server_comm.h"
#include "platform/proto/checkpoint_info.pb.h"
#include "platform/proto/resdb.pb.h"
#include "interface/common/resdb_txn_accessor.h"
#include "platform/consensus/execution/transaction_executor.h"
#include <semaphore.h>

namespace resdb {

class CheckPointManager : public CheckPoint {
 public:
  CheckPointManager(const ResDBConfig& config,
                    ReplicaCommunicator* replica_communicator,
                    SignatureVerifier* verifier);
  virtual ~CheckPointManager();

  TxnMemoryDB* GetTxnDB();
  uint64_t GetMaxTxnSeq();

  void AddCommitData(std::unique_ptr<Request> request);
  int ProcessCheckPoint(std::unique_ptr<Context> context,
                        std::unique_ptr<Request> request);

  uint64_t GetStableCheckpoint() override;
  StableCheckPoint GetStableCheckpointWithVotes();
  bool IsValidCheckpointProof(const StableCheckPoint& stable_ckpt);

  void SetTimeoutHandler(std::function<void()> timeout_handler);
  virtual void UpdateStableCheckPointCallback(
      int64_t current_stable_checkpoint) {}

  void Stop();

  void TimeoutHandler();

  void WaitSignal();
  std::unique_ptr<std::pair<uint64_t, std::string>> PopStableSeqHash();

  void SetExecutor(TransactionExecutor* executor){
    executor_ = executor;
  }

  uint64_t GetHighestPreparedSeq();

  void SetHighestPreparedSeq(uint64_t seq);

  sem_t* CommitableSeqSignal();

  uint64_t GetCommittableSeq();

 private:
  void UpdateCheckPointStatus();
  void UpdateStableCheckPointStatus();
  void BroadcastCheckPoint(uint64_t seq, const std::string& hash, 
                            const std::vector<std::string>& stable_hashs, 
                            const std::vector<uint64_t>& stable_seqs);

  void Notify();
  bool Wait();

 protected:
  ResDBConfig config_;
  ReplicaCommunicator* replica_communicator_;
  std::unique_ptr<TxnMemoryDB> txn_db_;
  std::thread checkpoint_thread_, stable_checkpoint_thread_;
  SignatureVerifier* verifier_;
  std::atomic<bool> stop_;
  std::map<std::pair<uint64_t, std::string>, std::set<uint32_t>> sender_ckpt_;
  std::map<std::pair<uint64_t, std::string>, std::vector<SignatureInfo>>
      sign_ckpt_;
  std::map<std::pair<uint64_t, std::string>, std::vector<std::string>>
      hash_ckpt_;
  std::atomic<uint64_t> current_stable_seq_;
  std::mutex mutex_;
  LockFreeQueue<Request> data_queue_;
  std::mutex cv_mutex_;
  std::condition_variable cv_;
  std::function<void()> timeout_handler_;
  StableCheckPoint stable_ckpt_;
  int new_data_ = 0;
  LockFreeQueue<std::pair<uint64_t, std::string>> stable_hash_queue_;
  std::condition_variable signal_;
  ResDBTxnAccessor txn_accessor_;
  std::mutex lt_mutex_;
  uint64_t last_seq_ = 0;
  TransactionExecutor* executor_;
  std::atomic<uint64_t> highest_prepared_seq_;
  uint64_t committable_seq_ = 0;
  std::string last_hash_, committable_hash_;
  sem_t committable_seq_signal_;
};

}  // namespace resdb
