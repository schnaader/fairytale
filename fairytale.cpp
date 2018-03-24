#include "analyser.h"

struct Stats {
	uint64_t deduped, zlib, jpeg, img1, img4, img8, img8gray, img24, img32, text, dds, mod;
	struct {
		uint64_t zlib, jpeg, img1, img4, img8, img8gray, img24, img32, text, dds, mod;
	} totals;
} stats = { 0 };

void vliEncode(uint64_t i, FileStream* stream) {
	while (i > 0x7F) {
		stream->putChar(0x80 | (i & 0x7F));
		i >>= 7;
	}
	stream->putChar((uint8_t)i);
}

void assignIds(Block* block, int64_t* from) {
	do {
		block->id = (*from)++;
		if (block->child != nullptr)
			assignIds(block->child, from);
		block = block->next;
	} while (block != nullptr);
}

void dumpToFile(Block* block, StorageManager* manager, FileStream* stream) {
	uint8_t buffer[GENERIC_BUFFER_SIZE];
	do {
		vliEncode(block->id, stream);
		vliEncode(static_cast<uint32_t>(block->type), stream);
		bool wasDormant = false;

		switch (block->type) {
			case BlockType::DEDUP: {
				stats.deduped++;
				vliEncode(((Block*)block->info)->id, stream);
				break;
			}
			case BlockType::DEFLATE: {
				DeflateInfo* info = (DeflateInfo*)block->info;
				stream->putChar(info->zlibCombination);
				stream->putChar(info->zlibWindow);
				vliEncode(info->penaltyBytesUsed, stream);
				for (int i = 0; i < info->penaltyBytesUsed; i++)
					stream->putChar(info->penaltyBytes[i]);
				for (int i = 0; i <= info->penaltyBytesUsed; i++)
					vliEncode(info->posDiff[i], stream);
				stats.zlib++;
				stats.totals.zlib += block->length;
				break;
			}
			default: {
				switch (block->type) {
					case BlockType::IMAGE: {
						ImageInfo* info = (ImageInfo*)block->info;
						uint8_t c = (info->bpp * 2) | uint8_t(info->grayscale);
						stream->putChar(c);
						vliEncode(info->width, stream);
						vliEncode(info->height, stream);
						switch (info->bpp) {
							case 32: {
								stats.img32++;
								stats.totals.img32 += block->length;
								break;
							}
							case 24: {
								stats.img24++;
								stats.totals.img24 += block->length;
								break;
							}
							case 8: {
								if (info->grayscale)
									stats.img8gray++, stats.totals.img8gray += block->length;
								else
									stats.img8++, stats.totals.img8 += block->length;
								break;
							}
							case 4: {
								stats.img4++;
								stats.totals.img4 += block->length;
								break;
							}
							case 1: {
								stats.img1++;
								stats.totals.img1 += block->length;
								break;
							}
						}
						break;
					}
					case BlockType::AUDIO: {
						AudioInfo* info = (AudioInfo*)block->info;
						stream->putChar(info->mode);
						stats.mod++;
						stats.totals.mod += block->length;
						break;
					}
					case BlockType::DDS: {
						stats.dds++;
						stats.totals.dds += block->length;
						break;
					}
					case BlockType::JPEG: {
						stats.jpeg++;
						stats.totals.jpeg += block->length;
						break;
					}
					default: {}
				}
				vliEncode(block->length, stream);

				// attempt to revive stream if needed
				if (block->level > 0) {
					if (((HybridStream*)block->data)->wasPurged()) {
						if (!block->attemptRevival(manager))
							break;
					} else
						((HybridStream*)block->data)->setPurgeStatus(false);
				} else {
					if ((wasDormant = ((FileStream*)block->data)->dormant()) && !manager->wakeUp((FileStream*)block->data))
						break;
				}

				int64_t length = block->length;
				block->data->setPos(block->offset);
				while (length > 0) {
					size_t l = block->data->blockRead(&buffer[0], min(GENERIC_BUFFER_SIZE, length));
					if (l == 0)
						break;
					stream->blockWrite(&buffer[0], l);
					length -= l;
				}
			}
		}
		if (block->level > 0)
			((HybridStream*)block->data)->setPurgeStatus(true);
		else if (wasDormant)
			((FileStream*)block->data)->goToSleep();

		if (block->child != nullptr)
			dumpToFile(block->child, manager, stream);

		block = block->next;
	} while (block != nullptr);
}

int main(int argc, char** argv) {
#ifdef WINDOWS
	_setmaxstdio(2048);
#endif
	printf("Fairytale Prototype v0.016 by M. Pais, 2018\n\n");
	if (
	argc < 4 || argc > 7 || strlen(argv[1]) != 4 || (!isdigit(argv[1][0])) || (!isdigit(argv[1][1])) || (argv[1][0] > argv[1][1])
	|| ((argv[1][2] & 0xDF) != 'B') || ((argv[1][3] & 0xDF) != 'D')) {
		printf("Syntax: fairytale MTBD output_file input_files\n");
		printf("M: Memory cache coefficient, [0..9] -> [4MB..2048MB]\n");
		printf("T: Total storage coefficient, [0..9] -> [8MB..4096MB]\n");
		printf("B: Brute force DEFLATE streams [B -> on, b-> off]\n");
		printf("D: Perform deduplication stage [D -> on, d-> off]\n");
		printf("M<=T\n");
		getchar();
		return 0;
	}
	clock_t start_time = clock();
	FileStream output;
	output.create(argv[2]);

	Array<FileStream*> input(argc - 3);
	Block* block = new Block{ 0 };
	Block* root = block;
	for (int i = 3; i < argc; i++) {
		input[i - 3] = new FileStream;
		if (!input[i - 3]->open(argv[i])) {
			printf("File not found: %s\n", argv[i]);
			getchar();
			return 0;
		}
		block->data = input[i - 3];
		block->length = block->data->getSize();
		printf("Loaded %s, (%" PRIu64 " bytes), hashing... ", argv[i], block->length);
		block->calculateHash();
		printf("done\n");
		if (i > 3)
			input[i - 3]->goToSleep();
		if (i + 1 < argc) {
			block->next = new Block{ 0 };
			block = block->next;
		}
	}
	block->next = nullptr;
	block = root;

	StorageManager pool(1ull << (22 + (argv[1][0] & 0xF)), 1ull << (23 + (argv[1][1] & 0xF)));
	Deduper deduper;
	Array<Parsers> parsers(0);
	parsers.push_back(Parsers::JPEG_PROGRESSIVE);
	parsers.push_back((argv[1][2] == 'b') ? Parsers::DEFLATE : Parsers::DEFLATE_BRUTE);
	parsers.push_back(Parsers::BITMAP_NOHDR);
	parsers.push_back(Parsers::TEXT);
	parsers.push_back(Parsers::DDS);
	parsers.push_back(Parsers::MOD);
	Analyser analyser(&parsers);
	analyser.analyse(block, &pool, (argv[1][3] == 'D') ? &deduper : nullptr);

	int64_t id = 0;
	assignIds(block, &id);
	try {
		dumpToFile(block, &pool, &output);
	} catch (...) {
		printf("\n\nError writing output file!");
		return 0;
	}
	uint64_t total = output.getSize();
	printf("\n");
	if (stats.zlib > 0)
		printf("DEFLATE streams found: %" PRIu64 " (%" PRIu64 " bytes)\n", stats.zlib, stats.totals.zlib);
	if (stats.jpeg > 0)
		printf("JPEG streams found: %" PRIu64 "  (%" PRIu64 " bytes)\n", stats.jpeg, stats.totals.jpeg);
	if (stats.img32 + stats.img24 + stats.totals.img8 + stats.img4 + stats.img1 > 0)
		printf("Uncompressed image streams stats:\n");
	if (stats.img32 > 0)
		printf("%" PRIu64 " @32bpp (%" PRIu64 " bytes)\n", stats.img32, stats.totals.img32);
	if (stats.img24 > 0)
		printf("%" PRIu64 " @24bpp (%" PRIu64 " bytes)\n", stats.img24, stats.totals.img24);
	if (stats.totals.img8 > 0)
		printf("%" PRIu64 " @8bpp palette-indexed, %" PRIu64 " @8bpp grayscale (%" PRIu64 " bytes)\n", stats.img8, stats.img8gray, stats.totals.img8);
	if (stats.img4 > 0)
		printf("%" PRIu64 " @4bpp (%" PRIu64 " bytes)\n", stats.img4, stats.totals.img4);
	if (stats.img1 > 0)
		printf("%" PRIu64 " @1bpp (%" PRIu64 " bytes)\n", stats.img1, stats.totals.img1);
	if (stats.dds > 0)
		printf("DDS textures found: %" PRIu64 ", (%" PRIu64 " bytes)\n", stats.dds, stats.totals.dds);
	if (stats.mod > 0)
		printf("MOD audio streams found: %" PRIu64 ", (%" PRIu64 " bytes)\n", stats.mod, stats.totals.mod);
	if (stats.text > 0)
		printf("Text streams found: %" PRIu64 " (%" PRIu64 " bytes)\n\n", stats.text, stats.totals.text);
	printf("\nDone in %1.2f sec, %" PRIu64 " bytes, %" PRIu64 " blocks were deduped", double(clock() - start_time) / CLOCKS_PER_SEC, total, stats.deduped);
	getchar();
	return 0;
}
