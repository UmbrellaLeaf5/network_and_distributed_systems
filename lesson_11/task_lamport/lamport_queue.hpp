#pragma once

#include <array>
#include <iostream>
#include <set>

/// @brief Класс временной метки
class Timestamp {
 private:
  int l_time_;
  int rank_;

 public:
  /**
   * @brief Инициализирует новый экземпляр Timestamp без времени появления
   * @param l_time: лэмпортовское время отправки запроса на вход в критическую
   * секцию процессом
   * @param rank: ранг процесса
   */
  Timestamp(int l_time, int rank) : l_time_(l_time), rank_(rank) {}

  /// @return int: лэмпортовское время
  int L() const { return l_time_; }

  /// @return int: ранг процесса
  int Rank() const { return rank_; }

  bool operator<(const Timestamp& ts) const {
    // лексикографическое сравнение
    return l_time_ < ts.L() || (l_time_ == ts.L() && rank_ < ts.Rank());
  }
};

std::ostream& operator<<(std::ostream& os, const Timestamp& ts) {
  return os << "{" << ts.L() << ", " << ts.Rank() << "}";
}

/// @brief Класс очереди для алгоритма Лэмпорта
class LamportQueue {
  // IMP: возможно, тут не хватает проверок на то, что текущий ранг уже есть в
  // очереди, но так как для этого нужно пробегаться по ней всей в поисках, где
  // же есть такой ts, я оставляю адекватность использования на пользователя

 private:
  std::set<Timestamp> queue_;
  int ranks_amount_;

 public:
  /**
   * @brief Инициализирует новый экземпляр LamportQueue
   * @param ranks_amount: максимальное кол-во процессов в очереди
   */
  LamportQueue(int ranks_amount) : ranks_amount_(ranks_amount) {}

  /**
   * @brief Добавляет метку в очередь
   * @details В том случае, если ts.Rank() >= ranks_amount или curr_size
   * (текущий размер очереди) >= ranks_amount, ничего не делает,
   * выводя сообщение об ошибке в поток `std::cout`.
   * @param ts: временная метка
   */
  void Add(const Timestamp& ts) {
    if (ts.Rank() >= ranks_amount_) {
      std::cout << "ERROR: LamportQueue::Add: ts.Rank() >= ranks_amount"
                << "\n";
      return;
    }

    if (queue_.size() >= ranks_amount_) {
      std::cout << "ERROR: LamportQueue::Add: curr_size >= ranks_amount"
                << "\n";
      return;
    }

    queue_.insert(ts);
  }

  /**
   * @brief Удаляет первую метку процесса из очереди
   * @details В том случае, если ts.Rank() >= ranks_amount, ничего не делает,
   * выводя сообщение об ошибке в поток `std::cout`.
   * @param rank: ранг процесса
   */
  void Remove(int rank) {
    if (rank >= ranks_amount_) {
      std::cout << "ERROR: LamportQueue::Remove: rank >= ranks_amount"
                << "\n";
      return;
    }

    std::set<Timestamp>::iterator it = queue_.begin();
    while (it != queue_.end())
      if (it->Rank() == rank) {
        it = queue_.erase(it);
        break;  // дальше бегать нет смысла - ранг должен попадаться лишь 1 раз
      } else
        it++;
  }

  /**
   * @brief Проверяет, является ли процесс головой очереди
   * @param rank: ранг процесса
   * @return true: является
   * @return false: не является
   */
  bool IsHead(int rank) const {
    return !queue_.empty() && queue_.begin()->Rank() == rank;
  }

  friend std::ostream& operator<<(std::ostream& os, const LamportQueue& lq) {
    for (const auto& ts : lq.queue_) os << ts << "; ";
    return os;
  }
};
