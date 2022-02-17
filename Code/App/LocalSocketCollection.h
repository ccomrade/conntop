#pragma once

#include <cassert>
#include <algorithm>
#include <vector>

#include "LocalSocket.h"

class LocalSocketCollection : public std::vector<LocalSocket>
{
	struct CompareFunctor
	{
		bool operator()(const LocalSocket& a, const LocalSocket& b) const
		{
			return a.inode < b.inode;
		}

		bool operator()(const LocalSocket& socket, uint64_t inode) const
		{
			return socket.inode < inode;
		}
	};

public:
	LocalSocketCollection() = default;

	void sort()
	{
		std::sort(begin(), end(), CompareFunctor());
	}

	bool is_sorted() const
	{
		return std::is_sorted(begin(), end(), CompareFunctor());
	}

	iterator find(std::uint64_t inode)
	{
		assert(is_sorted());

		// binary search
		const auto it = std::lower_bound(begin(), end(), inode, CompareFunctor());

		return (it != end() && it->inode == inode) ? it : end();
	}

	const_iterator find(std::uint64_t inode) const
	{
		assert(is_sorted());

		// binary search
		const auto it = std::lower_bound(begin(), end(), inode, CompareFunctor());

		return (it != end() && it->inode == inode) ? it : end();
	}
};
