import argparse
import subprocess
import sys

PACKAGE = "com.asterin.neowpf.android" # samples/Android/

LEVELS = {
    "error": "E",
    "warn": "W",
    "info": "I",
}


def get_pid():
    result = subprocess.run(
        ["adb", "shell", "pidof", PACKAGE],
        capture_output=True,
        text=True,
    )

    return result.stdout.strip().split()


def main():
    parser = argparse.ArgumentParser(
    )

    parser.add_argument("--error", action="store_true", help="Show error logs.")
    parser.add_argument("--warn", action="store_true", help="Show warning logs.")
    parser.add_argument("--info", action="store_true", help="Show info logs.")
    parser.add_argument(
        "--pid",
        action="store_true",
        help="Filter by the app's PID. Without this, listen to all processes.",
    )

    args = parser.parse_args()

    selected_levels = [
        level
        for name, level in LEVELS.items()
        if getattr(args, name)
    ]

    # No level specified means include every log level.
    if not selected_levels:
        selected_levels = ["V", "D", "I", "W", "E", "F", "S"]
    else:
        if "E" in selected_levels:
            selected_levels = ["E"]
        elif "W" in selected_levels:
            selected_levels = ["W", "E"]
        elif "I" in selected_levels:
            selected_levels = ["I", "W", "E"]

    command = ["adb", "logcat", "-v", "threadtime"]

    if args.pid:
        pids = get_pid()

        if not pids:
            print(f"Could not find a running process for {PACKAGE}.", file=sys.stderr)
            sys.exit(1)

        command.extend(["--pid", ",".join(pids)])

    command.extend(["*:S"])

    for level in selected_levels:
        command.append(f"*:{level}")

    print(f"Listening to {PACKAGE}")
    print(f"Levels: {', '.join(selected_levels)}")
    print(f"PID filtering: {'enabled' if args.pid else 'disabled'}")
    print("Press Ctrl+C to stop.\n")

    try:
        subprocess.run(command)
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()