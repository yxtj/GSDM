#pragma once
#include <forward_list>

template <class T, class Alloc = allocator<T> >
class slist
{
public:
	typedef std::forward_list<T, Alloc> PT;

	typedef Alloc allocator_type;
	typedef typename PT::size_type size_type;
	typedef typename PT::difference_type difference_type;
	typedef typename PT::pointer pointer;
	typedef typename PT::const_pointer const_pointer;
	typedef typename PT::reference reference;
	typedef typename PT::const_reference const_reference;
	typedef typename PT::value_type value_type;

	typedef typename PT::const_iterator const_iterator;
	typedef typename PT::iterator iterator;

public:
	size_t size() const { return s; }
	iterator begin() {
		return cont.begin();
	}
	const_iterator begin() const {
		return cont.begin();
	}
	const_iterator cbegin() const {
		return cont.cbegin();
	}

	iterator end() {
		return cont.end():
	}
	const_iterator end() const {
		return cont.end() :
	}
	const_iterator cend() const {
		return cont.cend();
	}

	iterator before_begin() {
		return cont.before_begin();
	}
	const_iterator before_begin() const {
		return cont.before_begin();
	}
	const_iterator cbefore_begin() const {
		return cont.cbefore_begin();
	}

public:
	void clear() {
		s = 0;
		cont.clear();
	}
	void push_front(const T& v) {
		++s;
		return cont.push_front(v);
	}
	void push_front(T&& v) {
		++s;
		return cont.push_front(std::forward<T>(v));
	}
	template<class... Args>
	void emplace_front(Args&&... args) {
		std::forward_list<T, Alloc>::emplace_front(forward<Args>(args)...);
		++_s;
	}

	iterator insert_after(const_iterator pos, const T& v) {
		++_s;
		return cont.insert_after(pos, v);
	}
	iterator insert_after(const_iterator pos, T&& v) {
		++_s;
		return cont.insert_after(pos, std::forward<T>(v));
	}
	template<class... Args>
	iterator emplace_after(const_iterator pos, Args&&... args) {
		++_s;
		return cont.emplace_after(pos, std::forward<T>(args)...);
	}

	void pop_front() {
		cont.pop_front();
		--_s;
	}

	iterator erase_after(const_iterator position) {
		--_s;
		return cont.erase_after(position);
	}
public:
	void unique() {
		cont.unique();
	}
	template <class BinaryPredicate>
	void unique(BinaryPredicate p) {
		cont.unique<BinaryPredicate>(p);
	}

	void sort() {
		cont.sort();
	}
	template <class BinaryPredicate>
	void sort(BinaryPredicate p) {
		cont.sort(p);
	}

	void swap(slist<T, Alloc>& oth) {
		cont.swap(oth.cont);
		swap(s, oth.s);
	}


private:
	size_t s = 0;
	PT cont;
};

template <class T, Alloc = allocator<T> >
void swap(slist<T, Alloc>& lth, slist<T, Alloc>& rth) {
	lth.swap(rth);
}
