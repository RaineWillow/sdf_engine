#pragma once
#ifndef Thread_Safe_Memcpy_hpp
#define Thread_Safe_Memcpy_hpp
#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class SafeMemcpy {
public:

private:
  std::queue<T> m_queue;
  std::mutex m_mutex;
  std::condition_variable m_cond;
};

#endif