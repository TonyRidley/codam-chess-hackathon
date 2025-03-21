#include <search.h>
#include <stdio.h>
#include <stdlib.h>

struct opening_book* load_opening_book(const char *filename) {
	
	FILE *f = fopen(filename, "rb");
	if (!f) {
		return NULL;
	}
	
	struct opening_book *book = malloc(sizeof(struct opening_book));
	if (!book) {
		fclose(f);
		return NULL;
	}

	// Read file size and allocate entries
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);
	
	book->num_entries = size / sizeof(struct book_entry);
	
	book->entries = malloc(size);
	book->max_ply = 20;  // We only store first 10 moves
	
	if (!book->entries) {
		free(book);
		fclose(f);
		return NULL;
	}
	
	// Read all entries
	size_t entries_read = fread(book->entries, sizeof(struct book_entry), book->num_entries, f);
	if (entries_read != book->num_entries) {
		free(book->entries);
		free(book);
		fclose(f);
		return NULL;
	}
	
	printf("[DEBUG] Successfully loaded opening book with %zu entries\n", book->num_entries);
	fclose(f);
	return book;
}

struct move get_book_move(const struct position *pos, const struct opening_book *book, int current_ply) {
	struct move empty_move = {0, 0, 0};
	
	if (!book || !book->entries) {
		return empty_move;
	}
	
	if (current_ply >= book->max_ply * 2) {
		return empty_move;
	}
	
	__uint64_t pos_hash = get_hash(pos);
	
	// Binary search in the book
	int left = 0;
	int right = book->num_entries - 1;
	unsigned int best_count = 0;
	struct move best_move = empty_move;
	int found_index = -1;
	
	// First, find any matching position
	while (left <= right) {
		int mid = (left + right) / 2;
		
		if (book->entries[mid].hash == pos_hash) {
			found_index = mid;
			break;
		}
		
		if (book->entries[mid].hash < pos_hash)
			left = mid + 1;
		else
			right = mid - 1;
	}
	
	// If we found a matching position, look for all entries with the same hash
	if (found_index != -1) {
		size_t current = found_index;
		while (current >= 0 && book->entries[current].hash == pos_hash) {
			if (book->entries[current].count > best_count) {
				best_count = book->entries[current].count;
				best_move.from_square = book->entries[current].from_square;
				best_move.to_square = book->entries[current].to_square;
				best_move.promotion_type = -1;
				
				if (pos->board[best_move.from_square] == PAWN && RANK(best_move.to_square) == 7) {
					best_move.promotion_type = QUEEN;
				}
				else if (pos->board[best_move.from_square] == PAWN && RANK(best_move.to_square) == 0) {
					best_move.promotion_type = QUEEN;
				}
			}
			current--;
		}
		
		// Look forwards for entries with the same hash
		current = found_index + 1;
		while (current < book->num_entries && book->entries[current].hash == pos_hash) {
			if (book->entries[current].count > best_count) {
				best_count = book->entries[current].count;
				best_move.from_square = book->entries[current].from_square;
				best_move.to_square = book->entries[current].to_square;
				best_move.promotion_type = 0; 
				
				if (pos->board[best_move.from_square] == PAWN && RANK(best_move.to_square) == 7) {
					best_move.promotion_type = QUEEN;
				}
				else if (pos->board[best_move.from_square] == PAWN && RANK(best_move.to_square) == 0) {
					best_move.promotion_type = QUEEN;
				}
			}
			current++;
		}
		return best_move;
	}
	return empty_move;
}
