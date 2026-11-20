
import argparse
import os
import subprocess
import sys


def git(*args: str, input: str | None = None) -> str:
    result = subprocess.run(
        ["git", *args],
        input=input,
        capture_output=True,
        text=True,
    )

    if result.returncode != 0:
        raise RuntimeError(result.stderr.strip())

    return result.stdout.strip()


def get_commit(commit: str) -> dict[str, str]:
    format_string = (
        "%H%n%T%n%P%n%an%n%ae%n%aI%n%cn%n%ce%n%cI%n%B"
    )

    fields = git(
        "show", "-s", f"--format={format_string}", commit
    ).split("\n", 10)

    return {
        "hash": fields[0],
        "tree": fields[1],
        "parents": fields[2],
        "author_name": fields[3],
        "author_email": fields[4],
        "author_date": fields[5],
        "committer_name": fields[6],
        "committer_email": fields[7],
        "committer_date": fields[8],
        "message": fields[9],
    }


def squash(source: str, target: str) -> str:
    source_commit = get_commit(source)
    target_commit = get_commit(target)

    if target_commit["hash"] not in git(
        "rev-list", source_commit["hash"]
    ).splitlines():
        raise RuntimeError(
            "The target commit must be an ancestor of the source commit."
        )

    parents = target_commit["parents"].split()

    if len(parents) > 1:
        raise RuntimeError("Merge commits are not supported.")

    args = ["commit-tree", source_commit["tree"]]

    if parents:
        args.extend(["-p", parents[0]])

    env = {
        "GIT_AUTHOR_NAME": target_commit["author_name"],
        "GIT_AUTHOR_EMAIL": target_commit["author_email"],
        "GIT_AUTHOR_DATE": target_commit["author_date"],
        "GIT_COMMITTER_NAME": target_commit["committer_name"],
        "GIT_COMMITTER_EMAIL": target_commit["committer_email"],
        "GIT_COMMITTER_DATE": target_commit["committer_date"],
    }

    result = subprocess.run(
        ["git", *args],
        input=target_commit["message"],
        capture_output=True,
        text=True,
        env={**os.environ, **env},
    )

    if result.returncode != 0:
        raise RuntimeError(result.stderr.strip())

    new_commit = result.stdout.strip()

    git("reset", "--soft", new_commit)

    return new_commit


def commit(message: str, no_verify: bool) -> str:
    git("add", "-A")

    args = ["commit"]

    if no_verify:
        args.append("--no-verify")

    args.extend(["-m", message])
    git(*args)

    return git("rev-parse", "HEAD")


def main():
    parser = argparse.ArgumentParser(
        description="Git commit and history utilities."
    )

    operation = parser.add_mutually_exclusive_group(required=True)

    operation.add_argument(
        "--squash",
        nargs=2,
        metavar=("SOURCE", "TARGET"),
        help="Squash SOURCE into TARGET, preserving TARGET metadata.",
    )

    operation.add_argument(
        "--commit",
        metavar="MESSAGE",
        help="Commit all current changes using MESSAGE.",
    )

    parser.add_argument(
        "--no-verify",
        action="store_true",
        help="Skip Git hooks when committing.",
    )

    args = parser.parse_args()

    try:
        if args.squash:
            if args.no_verify:
                raise RuntimeError(
                    "--no-verify can only be used with --commit."
                )

            source, target = args.squash

            if git("rev-parse", "HEAD") != git("rev-parse", source):
                raise RuntimeError("Source commit must be HEAD.")

            result = squash(source, target)

            print(f"Squashed into: {result}")
            print("Current branch updated.")

        elif args.commit is not None:
            result = commit(args.commit, args.no_verify)
            print(f"Committed: {result}")

    except RuntimeError as error:
        print(f"Error: {error}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
