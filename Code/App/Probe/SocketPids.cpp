#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <memory>
#include <string>
#include <string_view>
#include <charconv>

#include "App/LocalSocketCollection.h"
#include "System/System.h"

#include "SocketPids.h"

namespace
{
	struct DIR_Deleter
	{
		void operator()(DIR* dir) const
		{
			closedir(dir);
		}
	};

	using DIR_Wrapper = std::unique_ptr<DIR, DIR_Deleter>;

	DIR_Wrapper OpenDirectory(const std::string& path)
	{
		auto dir = DIR_Wrapper(opendir(path.c_str()));
		if (!dir)
		{
			throw System::Error("Failed to open directory '{}'", path);
		}

		return dir;
	}

	bool ParsePid(const std::string_view& text, std::uint32_t& pid)
	{
		const char* first = text.data();
		const char* last = first + text.length();

		const auto status = std::from_chars(first, last, pid);

		return status.ec == std::errc() && status.ptr == last;
	}

	bool ParseSocketInode(const std::string_view& text, std::uint64_t& inode)
	{
		static constexpr std::string_view PREFIX = "socket:[";

		// make sure the text contains at least one additional character and begins with the prefix
		if (text.length() <= PREFIX.length() || std::string_view(text.data(), PREFIX.length()) != PREFIX)
			return false;

		const char* first = text.data() + PREFIX.length();
		const char* last = first + text.length() - 1;  // skip ']' at the end

		if (*last != ']')
			return false;

		const auto status = std::from_chars(first, last, inode);

		return status.ec == std::errc() && status.ptr == last;
	}

	bool ReadAndParseSocketInode(const std::string& path, std::uint64_t& inode)
	{
		char buffer[64];
		const ssize_t length = readlink(path.c_str(), buffer, sizeof buffer);
		if (length < 0)
		{
			throw System::Error("Failed to read symlink '{}'", path);
		}

		return ParseSocketInode(std::string_view(buffer, length), inode);
	}

	void AddSocketPid(LocalSocketCollection& sockets, std::uint64_t inode, std::uint32_t pid)
	{
		const auto it = sockets.find(inode);
		if (it != sockets.end())
		{
			it->pids.push_back(pid);
		}
	}
}

void SocketPids::Resolve(LocalSocketCollection& sockets)
{
	for (LocalSocket& socket : sockets)
	{
		socket.pids.clear();
	}

	std::string path = "/proc";

	const auto procDir = OpenDirectory(path);

	path += '/';
	const auto procBasePathLength = path.length();

	dirent* procDirEntry;
	while ((procDirEntry = readdir(procDir.get())) != nullptr)
	{
		std::uint32_t pid = 0;

		// only process directories
		if (procDirEntry->d_type & DT_DIR && ParsePid(procDirEntry->d_name, pid))
		{
			// keep "/proc/" and add process ID
			path.replace(procBasePathLength, path.length(), procDirEntry->d_name);
			path += "/fd";

			const auto fdDir = OpenDirectory(path);

			path += '/';
			const auto fdBasePathLength = path.length();

			dirent* fdDirEntry;
			while ((fdDirEntry = readdir(fdDir.get())) != nullptr)
			{
				// only symlinks
				if (fdDirEntry->d_type & DT_LNK)
				{
					// keep "/proc/123/fd/" and add file descriptor number
					path.replace(fdBasePathLength, path.length(), fdDirEntry->d_name);

					std::uint64_t inode = 0;

					if (ReadAndParseSocketInode(path, inode))
					{
						AddSocketPid(sockets, inode, pid);
					}
				}
			}
		}
	}
}
