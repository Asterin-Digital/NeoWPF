import argparse
import subprocess
import sys

def git(*args, capture_output=False):
    result = subprocess.run(
        ["git", *args],
        text=True,
        capture_output=capture_output
    )

    if result.returncode != 0:
        if capture_output and result.stderr:
            print(result.stderr.strip(), file=sys.stderr)
        raise RuntimeError(f"Git command failed: git {' '.join(args)}")

    return result.stdout.strip() if capture_output else None


def main():
    parser = argparse.ArgumentParser(
        description="Commit local changes and force-push to the remote repository."
    )
    parser.add_argument(
        "commit_message",
        nargs="?",
        help="Commit message, requested only when there are local changes."
    )
    parser.add_argument(
        "--remote",
        default="origin",
        help="Git remote to push to (default: origin)."
    )
    parser.add_argument(
        "--branch",
        help="Remote branch to overwrite (defaults to the current local branch)."
    )

    args = parser.parse_args()

    try:

        repo_root = git("rev-parse", "--show-toplevel", capture_output=True)
        subprocess.run(["git", "-C", repo_root, "status"], stdout=subprocess.DEVNULL)


        local_branch = git(
            "branch", "--show-current", capture_output=True
        )

        if not local_branch:
            raise RuntimeError("Cannot push while in a detached HEAD state.")

        target_branch = args.branch or local_branch


        remote_url = git(
            "remote", "get-url", args.remote, capture_output=True
        )

        print(f"Repository: {repo_root}")
        print(f"Remote:     {args.remote} ({remote_url})")
        print(f"Branch:     {local_branch} -> {target_branch}")


        changes = git(
            "status", "--porcelain", "--untracked-files=all",
            capture_output=True
        )

        if changes:
            commit_message = args.commit_message

            if not commit_message:
                commit_message = input("Commit message: ").strip()

            if not commit_message:
                raise RuntimeError("Commit message cannot be empty.")

        print()
        print("WARNING: This will forcefully overwrite the remote branch.")
        print("Remote commits not present in your local branch will be lost.")
        print()

        confirmation = input(
            f"Type 'overwrite' to continue: "
        ).strip()

        if confirmation != "overwrite":
            print("Cancelled.")
            return


        if changes:
            print("\nStaging changes...")
            git("add", "--all")

            print("Creating commit...")
            git("commit", "-m", commit_message)
        else:
            print("\nNo changes detected. Skipping commit.")


        print(f"\nForce-pushing to {args.remote}/{target_branch}...")

        git(
            "push",
            "--force",
            "--set-upstream",
            args.remote,
            f"HEAD:refs/heads/{target_branch}"
        )

        print("\nSuccessfully overwrote the remote branch.")

    except (RuntimeError, KeyboardInterrupt) as error:
        print(f"\nError: {error}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()