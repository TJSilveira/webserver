/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerCore.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tiago <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 22:41:05 by tiago             #+#    #+#             */
/*   Updated: 2026/01/26 22:42:56 by tiago            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCORE_HPP
#define SERVERCORE_HPP
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>

class ServerCore {
private:
public:
  std::vector<int> _vec;
  std::deque<int> _deque;

  static int number_of_operations;
  // Constructors
  ServerCore(void);
  ServerCore(const PmergeMe &other);
  ServerCore(int &argc, char *argv[]);

  // Destructor
  ~ServerCore(void);

  // Getter

  // Methods
  void intialize_vector(int &argc, char **argv);
  void intialize_deque(int &argc, char **argv);
  void merge_insertion_sort_vec(int pair_level);
  int sort_pairs_vec(int rec_depth);
  void initialize_main_pend_vec(int rec_depth, std::vector<int> &main,
                                std::vector<int> &pend,
                                std::vector<int> &non_pend);

  void merge_insertion_sort_deque(int pair_level);
  int sort_pairs_deque(int rec_depth);
  void initialize_main_pend_deque(int rec_depth, std::deque<int> &main,
                                  std::deque<int> &pend,
                                  std::deque<int> &non_pend);

  void is_sorted_vec();
  void is_sorted_deque();

  // Overloaded Operator
  PmergeMe &operator=(const PmergeMe &other);

  // Exceptions
  class MissingInput : public std::exception {
  public:
    virtual const char *what() const throw();
  };

  class InvalidNumber : public std::exception {
  public:
    virtual const char *what() const throw();
  };

  class NotSorted : public std::exception {
  public:
    virtual const char *what() const throw();
  };
};

#endif
