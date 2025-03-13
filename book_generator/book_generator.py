import chess.pgn
import chess
import os
import gc
import psutil
import sys

from tqdm import tqdm

# Constants: -1 is used to represent no piece, while 12 = total piece variants (6 white + 6 black).
NO_PIECE = -1
NUM_PIECES = 12
ZOBRIST_KEYS = []


class OpeningBookEntry:
    def __init__(self):
        self.hash = 0  # 64-bit unsigned integer (__uint64_t)
        self.from_square = 0  # 16-bit unsigned integer (uint16_t)
        self.to_square = 0  # 16-bit unsigned integer (uint16_t)
        self.count = 0  # 16-bit unsigned integer (uint16_t)
        self.side = 0


class OpeningBookEntry:
    def __init__(self):
        self.hash = 0
        self.from_square = 0
        self.to_square = 0
        self.count = 0
        self.side = 0


class OpeningBook:
    def __init__(self):
        self.positions = {}

    def add_game(self, game, max_ply=10):
        board = chess.Board()
        moves = list(game.mainline_moves())

        for move_num, move in enumerate(moves):
            if move_num >= max_ply:
                break

            pos_hash = get_hash(board)

            # Skip move validation and directly apply the move
            board.push(move)

            if pos_hash not in self.positions:
                entry = OpeningBookEntry()
                entry.hash = pos_hash
                entry.from_square = move.from_square
                entry.to_square = move.to_square
                entry.count = 1
                self.positions[pos_hash] = entry
            else:
                entry = self.positions[pos_hash]
                if entry.from_square == move.from_square and entry.to_square == move.to_square:
                    entry.count += 1

    def save_to_binary(self, filename):
        sorted_positions = sorted(self.positions.items(), key=lambda x: x[0])

        with open(filename, 'wb') as f:
            for pos_hash, entry in sorted_positions:
                f.write(pos_hash.to_bytes(8, byteorder='little'))
                f.write(entry.from_square.to_bytes(2, byteorder='little'))
                f.write(entry.to_square.to_bytes(2, byteorder='little'))
                f.write(entry.count.to_bytes(4, byteorder='little'))
                f.write(entry.side.to_bytes(1, byteorder='little'))
                f.write(b'\0')  # Padding byte


def get_memory_usage():
    process = psutil.Process(os.getpid())
    return process.memory_info().rss / 1024 / 1024

def load_zobrist_keys():
    """
    Load pre-generated Zobrist keys from text file.
    """
    global ZOBRIST_KEYS
    ZOBRIST_KEYS.clear()

    with open("zobrist_keys.txt", "r") as f:
        # Remove "ULL" suffix and convert each line to integer
        ZOBRIST_KEYS.extend(int(line.replace("ULL", "")) for line in f.read().split(",\n"))

def piece_to_index(piece):
    """
    Convert a python-chess Piece object to an integer index in [0..11].
    White pieces occupy [0..5], black pieces occupy [6..11].
    piece_type: Pawn=1, Knight=2, Bishop=3, Rook=4, Queen=5, King=6.
    """
    if piece is None:
        return NO_PIECE

    # Offset is 0 for white, 6 for black.
    color_offset = 0 if piece.color == chess.WHITE else 6

    # Subtract 1 because chess.Piece.piece_type goes from 1..6,
    # while we want an index 0..5 for each color.
    type_index = piece.piece_type - 1

    return color_offset + type_index


def get_hash(board):
    """
    Computes the Zobrist hash of a chess.Board object using a single-array
    approach: ZOBRIST_KEYS[square * NUM_PIECES + piece_index].
    """
    h = 0
    for square in range(64):
        piece = board.piece_at(square)
        index = piece_to_index(piece)
        if index != NO_PIECE:
            z_index = square * NUM_PIECES + index
            h ^= ZOBRIST_KEYS[z_index]
    return h


def process_pgn_file(pgn_file, book):
    games_processed = 0
    file_size = os.path.getsize(pgn_file)

    with open(pgn_file) as f:
        pbar = tqdm(total=file_size,
                    desc=f"Processing {os.path.basename(pgn_file)}",
                    unit='B',
                    unit_scale=True,
                    unit_divisor=1024)

        last_pos = 0
        while True:
            try:
                game = chess.pgn.read_game(f)
                if game is None:
                    break

                current_pos = f.tell()
                pbar.update(current_pos - last_pos)
                last_pos = current_pos

                book.add_game(game)
                games_processed += 1

                if games_processed % 1000 == 0:
                    pbar.set_postfix({'Games': games_processed})

            except Exception as e:
                continue

        pbar.close()

    return games_processed


def main():
    pgn_directory = "pgn_files"
    binary_file = "../opening_book.bin"
    book = OpeningBook()

    load_zobrist_keys()

    total_processed = 0

    # Get list of PGN files and sort by size
    pgn_files = [(f, os.path.getsize(os.path.join(pgn_directory, f)))
                 for f in os.listdir(pgn_directory)
                 if f.endswith('.pgn')]
    pgn_files.sort(key=lambda x: x[1])

    print("Processing PGN files:")
    for pgn_file, size in pgn_files:
        filepath = os.path.join(pgn_directory, pgn_file)
        print(f"\nProcessing {pgn_file} ({size / (1024 * 1024):.2f} MB)")

        processed = process_pgn_file(filepath, book)
        total_processed += processed

        if get_memory_usage() > 1000:  # If memory usage exceeds 1GB
            gc.collect()  # Force garbage collection

        print(f"File statistics:")
        print(f"- Games processed: {processed}")
        print(f"- Current book positions: {len(book.positions)}")

    print("\nProcessing complete!")
    print(f"Total games processed: {total_processed}")
    print(f"Total positions in book: {len(book.positions)}")

    print("\nSaving opening book...")
    book.save_to_binary(binary_file)
    print(f"Opening book saved to {binary_file}")


if __name__ == "__main__":
    main()

