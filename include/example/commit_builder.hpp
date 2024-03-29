#pragma once

#include "../magic_builder/magic_builder.hpp"

#include <concepts>
#include <chrono>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

struct CommitData {
    std::string message;
    std::vector<std::filesystem::path> included_paths;
    std::optional<std::chrono::time_point<std::chrono::system_clock>> timestamp;
    std::vector<std::string> authors;
    std::optional<std::string> fixup_hash;
    std::optional<std::string> squash_hash;
};

struct CommitValidator : magic_bldr::Validator<CommitValidator> {
    enum class Action {
        AddMessage,
        AddFolder,
        AddFile,
        AddAll,
        SetTimestamp,
        SetTimestampNow,
        SetAuthorSelf,
        SetAuthor,
        SetMultipleAuthors,
        SetFixup,
        SetSquash,
    };

    bool has_message = false;
    bool has_adds = false;
    bool has_added_all = false;
    bool has_timestamp = false;
    bool has_author = false;
    bool has_fixup_or_squash = false;

    consteval bool ready() const {
        return has_message && has_adds && has_timestamp && has_author;
    };

    consteval CommitValidator state_after(Action a) const {
        using enum Action;
        using CV = CommitValidator;
        switch (a) {
            case AddMessage:
                return set(&CV::has_message, true);

            case AddFolder:
            case AddFile:
                return set(&CV::has_adds, true);

            case AddAll:
                return set(&CV::has_adds, true) + set(&CV::has_added_all, true);

            case SetTimestamp:
            case SetTimestampNow:
                return set(&CV::has_timestamp, true);

            case SetAuthor:
            case SetAuthorSelf:
            case SetMultipleAuthors:
                return set(&CV::has_author, true);

            case SetFixup:
            case SetSquash:
                return set(&CV::has_fixup_or_squash, true);
        }
    }
};


class Commit : CommitData {};

template <CommitValidator V = CommitValidator{}>
class CommitBuilder : private magic_bldr::Builder<Commit, CommitValidator, CommitData, CommitBuilder, V> {
  private:
    using MB = magic_bldr::Builder<Commit, CommitValidator, CommitData, CommitBuilder, V>;

  public:
    CommitBuilder() = default;
    CommitBuilder(MB mb) : MB{mb} {}

    auto add_message(std::string&& msg) && {
        return MB::template run<MB::Action::AddMessage>(std::move(msg));
    }

    auto add_folder(std::filesystem::path&& folder_path) && {
        return MB::template run<MB::Action::AddFolder>(std::move(folder_path));
    }

    auto add_file(std::filesystem::path&& file_path) && {
        return MB::template run<MB::Action::AddFile>(std::move(file_path));
    }

    auto add_all() && {
        return MB::template run<MB::Action::AddAll>();
    }

    auto set_timestamp(std::chrono::time_point<std::chrono::system_clock>&& ts) && {
        return MB::template run<MB::Action::SetTimestamp>(ts);
    }

    auto set_timestamp_now() && {
        return MB::template run<MB::Action::SetTimestampNow>();
    }

    auto set_author_self() && {
        return MB::template run<MB::Action::SetAuthorSelf>();
    }

    auto set_author(std::string&& author) && {
        return MB::template run<MB::Action::SetAuthor>(std::move(author));
    }

    template <std::convertible_to<std::string>... Ts>
    auto set_multiple_authors(Ts&&... authors) && {
        return MB::template run<MB::Action::SetAuthor>(std::move(authors)...);
    }

    auto set_fixup_hash(std::string&& hash) && {
        return MB::template run<MB::Action::SetFixup>(std::move(hash));
    }

    auto set_squash_hash(std::string&& hash) && {
        return MB::template run<MB::Action::SetFixup>(std::move(hash));
    }
};

#include "commit_builder_actionimpls.hpp"