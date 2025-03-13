import random

NUM_SQUARES = 64
NUM_PIECES = 12
ZOBRIST_ARRAY_SIZE = NUM_SQUARES * NUM_PIECES


def generate_zobrist_keys():
    random.seed(1234)
    # Generate and sort the keys
    keys = [random.getrandbits(64) for _ in range(ZOBRIST_ARRAY_SIZE)]
    return sorted(keys)  # Sort the keys in ascending order


def main():
    keys = generate_zobrist_keys()

    # Write sorted keys to file
    with open("zobrist_keys.txt", "w") as f:
        # Write them all in one line, separated by commas and ULL
        f.write("ULL,\n".join(str(key) for key in keys))

    # Debug output to verify sorting
    print(f"Generated {len(keys)} sorted Zobrist keys")
    print("First 5 keys:")
    for i in range(min(5, len(keys))):
        print(f"{keys[i]}ULL")
    print("\nLast 5 keys:")
    for i in range(max(0, len(keys) - 5), len(keys)):
        print(f"{keys[i]}ULL")


if __name__ == "__main__":
    main()
