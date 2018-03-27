#include "analyser.h"
#include "contrib/CLI11/CLI11.hpp"
#include "contrib/spdlog/spdlog.h"

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
          }
          else
            ((HybridStream*)block->data)->setPurgeStatus(false);
        }
        else {
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
  CLI::App app{ "Fairytale Prototype v0.016 by M. Pais, 2018" };
  int memory = 9;
  int total_storage = 9;
  bool brute_mode = false;
  bool deduplication = false;
  int verbose = 0;
  std::string output_file;
  std::vector<std::string> input_files;
  app.add_option("-m,--memory", memory, "Memory cache coefficient [4MB..2048MB]", 9)->check(CLI::Range(0, 9));
  app.add_option("-t,--total-storage", total_storage, "Total storage coefficient [8MB..4096MB]", 9)->check(CLI::Range(0, 9));
  app.add_flag("-b,--brute", brute_mode, "Brute force DEFLATE streams");
  app.add_flag("-d,--deduplication", deduplication, "Perform deduplication stage");
  app.add_flag("-v,--verbose", verbose, "Enable verbose output. Specify twice for more verbosity.");
  app.add_option("output_file,-o", output_file, "output_file")->required();
  app.add_option("input_files,-i,", input_files, "input_files")->required();
  CLI11_PARSE(app, argc, argv);

  auto console = spdlog::stdout_color_mt("console");
  spdlog::set_pattern("%v");
  switch (verbose) {
    case 0: spdlog::set_level(spdlog::level::info); break;
    case 1: spdlog::set_level(spdlog::level::debug); break;
    case 2: spdlog::set_level(spdlog::level::trace); break;
    default: spdlog::set_level(spdlog::level::info); break;
  }

  clock_t start_time = clock();
  FileStream output;
  output.create(output_file.c_str());

  Array<FileStream*> input(input_files.size());
  Block* block = new Block{ 0 };
  Block* root = block;
  for (size_t i = 0; i < input_files.size(); i++) {
    input[i] = new FileStream;
    if (!input[i]->open(input_files[i].c_str())) {
      console->critical("File not found: {0}", input_files[i]);
      getchar();
      return 0;
    }
    block->data = input[i];
    block->length = block->data->getSize();
    console->info("Loaded {0}, ({1} bytes), hashing... ", input_files[i], block->length);
    block->calculateHash();
    console->info("Done hashing.");
    if (i > 0)
      input[i]->goToSleep();
    if (i + 1 < input_files.size()) {
      block->next = new Block{ 0 };
      block = block->next;
    }
  }
  block->next = nullptr;
  block = root;

  StorageManager pool(1ull << (22 + memory), 1ull << (23 + total_storage));
  Deduper deduper;
  Array<Parsers> parsers(0);
  parsers.push_back(Parsers::JPEG_PROGRESSIVE);
  parsers.push_back(brute_mode ? Parsers::DEFLATE_BRUTE : Parsers::DEFLATE);
  parsers.push_back(Parsers::BITMAP_NOHDR);
  parsers.push_back(Parsers::TEXT);
  parsers.push_back(Parsers::DDS);
  parsers.push_back(Parsers::MOD);
  Analyser analyser(&parsers);
  analyser.analyse(block, &pool, deduplication ? &deduper : nullptr);

  int64_t id = 0;
  assignIds(block, &id);
  try {
    dumpToFile(block, &pool, &output);
  }
  catch (...) {
    console->critical("Error writing output file!");
    return 0;
  }
  uint64_t total = output.getSize();
  if (stats.zlib > 0)
    console->info("DEFLATE streams found: {0} ({1} bytes)", stats.zlib, stats.totals.zlib);
  if (stats.jpeg > 0)
    console->info("JPEG streams found: {0} ({1} bytes)", stats.jpeg, stats.totals.jpeg);
  if (stats.img32 + stats.img24 + stats.totals.img8 + stats.img4 + stats.img1 > 0)
    console->info("Uncompressed image streams stats:");
  if (stats.img32 > 0)
    console->info("{0} @32bpp ({1} bytes)", stats.img32, stats.totals.img32);
  if (stats.img24 > 0)
    console->info("{0} @24bpp ({1} bytes)", stats.img24, stats.totals.img24);
  if (stats.totals.img8 > 0)
    console->info("{0} @8bpp palette-indexed, {1} @8bpp grayscale ({2} bytes)", stats.img8, stats.img8gray, stats.totals.img8);
  if (stats.img4 > 0)
    console->info("{0} @4bpp ({1} bytes)", stats.img4, stats.totals.img4);
  if (stats.img1 > 0)
    console->info("{0} @1bpp ({1} bytes)", stats.img1, stats.totals.img1);
  if (stats.dds > 0)
    console->info("DDS textures found: {0}, ({1} bytes)", stats.dds, stats.totals.dds);
  if (stats.mod > 0)
    console->info("MOD audio streams found: {0}, ({1} bytes)", stats.mod, stats.totals.mod);
  if (stats.text > 0)
    console->info("Text streams found: {0} ({1} bytes)", stats.text, stats.totals.text);
  console->info("Done in {0:.2f} sec, {1} bytes, {2} blocks were deduped", double(clock() - start_time) / CLOCKS_PER_SEC, total, stats.deduped);
  getchar();
  return 0;
}
