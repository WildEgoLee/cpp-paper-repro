// Minimal Raft interfaces.
// 3-node cluster, term, vote, log.

#pragma once
#include <cstdint>
#include <vector>
#include <string>

enum class Role { Follower, Candidate, Leader };

struct LogEntry {
  uint64_t term;
  std::string command;
};

class RaftNode {
public:
  explicit RaftNode(int id);

  // RPCs
  bool request_vote(uint64_t term, int candidate_id);
  bool append_entries(uint64_t term, const std::vector<LogEntry>& entries);

  // Local
  void start_election();
  bool is_leader() const;
  uint64_t current_term() const;

private:
  int id_;
  Role role_ = Role::Follower;
  uint64_t current_term_ = 0;
  int voted_for_ = -1;
  std::vector<LogEntry> log_;
  uint64_t commit_index_ = 0;
};
