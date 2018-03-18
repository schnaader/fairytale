/*
  This file is part of the Fairytale project

  Copyright (C) 2018 Márcio Pais

  This library is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "filestream.h"

FileStream::FileStream() { file = nullptr, name = nullptr; }

FileStream::~FileStream() { close(); }

bool FileStream::open(const char *filename) {
  assert(file==nullptr);
  if (name==nullptr) {
    size_t len = strlen(filename)+1;
    name = new char[len]();
    memcpy(name, filename, len);
  }
#ifdef WINDOWS
  return fopen_s(&file, filename, "rb")==0;
#else
  return (file = fopen(filename, "rb"))!=nullptr;
#endif
}

bool FileStream::create(const char *filename) {
  assert(file==nullptr);
  assert(name==nullptr);
#ifdef WINDOWS
  return fopen_s(&file, filename, "wb+")!=0;
#else
  return (file = fopen(filename, "wb+"))!=nullptr;
#endif
}

bool FileStream::getTempFile() {
  assert(file==nullptr);
#ifdef WINDOWS
  wchar_t szTempFileName[MAX_PATH];
  if (GetTempFileName(L".", L"tmp", 0, szTempFileName)==0)
    return false;
#if 1
  if (_wfopen_s(&file, szTempFileName, L"w+bTD")!=0)
    return false;
#else
  HANDLE hFile = CreateFile(szTempFileName, (GENERIC_READ|GENERIC_WRITE), 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE, NULL);
  if (hFile==INVALID_HANDLE_VALUE)
    return false;
  int nHandle = _open_osfhandle((intptr_t)hFile, _O_APPEND);
  if (nHandle == -1) {
    CloseHandle(hFile);
    return false;
  }
  if ((file = _wfdopen(nHandle, L"w+bTD"))==nullptr) {
    CloseHandle(hFile);
    return false;
  }
#endif
  return true;
#else
  return (file = tmpfile())!=nullptr;
#endif
}

void FileStream::close() {
  if (file)
    fclose(file);
  file = NULL;
  if (name) {
    delete name;
    name = nullptr;
  }
}

size_t FileStream::blockRead(void *ptr, const size_t count) { assert(file!=nullptr); return fread(ptr, 1, count, file); }

void FileStream::blockWrite(void *ptr, const size_t count) { assert(file!=nullptr); if (fwrite(ptr, 1, count, file)!=count) throw ExhaustedStorageException(); }

void FileStream::setPos(const off_t newpos) { assert(file!=nullptr); fseeko(file, newpos, SEEK_SET); }

void FileStream::setEnd() { assert(file!=nullptr); fseeko(file, 0, SEEK_END); }

off_t FileStream::curPos() { assert(file!=nullptr); return off_t(ftello(file)); }

bool FileStream::wakeUp() { assert(dormant()); return open(name); }

void FileStream::goToSleep() { assert(!dormant()); if (file) { fclose(file), file = NULL; } }

int64_t FileStream::getSize() {
  assert(file!=nullptr);
  off_t prev = curPos();
  setEnd();
  off_t size = curPos();
  setPos(prev);
  return (int64_t)size;
}