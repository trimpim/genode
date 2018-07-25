/*
 * \brief  Application to show RAM and CAP usage of components.
 * \author Pirmin Duss
 * \date   2018-07-24
 */

/*
 * Copyright (C) 2015-2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/heap.h>


namespace Init_state_monitor {

	template <typename T, Genode::size_t N>
	class Ring_buffer;
	using namespace Genode;
}


/**
 * Ring buffer
 *
 * \param  T  type of elements
 * \param  N  number of elements to store
 *
 * The ring buffer manages its elements as values.
 * When inserting an element, a copy of the element is
 * stored in the buffer. Hence, the ring buffer is suited
 * for simple plain-data element types.
 */
template <typename T, Genode::size_t N>
class Init_state_monitor::Ring_buffer
{
	private:

		enum { CAPACITY = N + 1 };

		size_t  _head     { 0 };
		size_t  _tail     { 0 };

		T       _data[CAPACITY];

	public:

		struct Buffer_empty : public Exception { };
		struct Doesnt_exist : public Exception { };

		Ring_buffer() { }

		/**
		 * Return if ring buffer is empty
		 */
		bool empty() const { return _tail == _head; }

		/**
		 * Return the number of stored elements.
		 */
		size_t data_points() const {
			return (_tail > _head) ?  (_tail - _head) :
		                              (CAPACITY - _head + _tail);
		}

		/**
		 * Place an element in the buffer.
		 * If the buffer is already full, the oldest element is
		 * overwritten.
		 */
		void push(T elem) {
			_data[_tail] = elem;
			_tail = (_tail + 1) % CAPACITY;
			if( _head == _tail) {
				_head = (_head + 1) % CAPACITY;
			}
		}

		/**
		 * Return the oldest stored element and remove it.
		 *
		 * \throw Buffer_empty  no elements are stored
		 */
		T pop() {
			if (_head == _tail) {
				throw Buffer_empty();
			}

			T res = _data[_head];
			_head = (_head + 1) % CAPACITY;
		}

		/**
		 * Look up the nth stored element
		 *
		 * \throw Doesnt_exist  element is not available
		 */
		T lookup(size_t nth) const {
			if (nth > data_points()) {
				throw Doesnt_exist();
			}

			return _data[(_head - 1 + nth) % CAPACITY];
		}
};
