// Copyright 2024 RavBot Authors. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "ravbot/providers/curl_raii.hpp"

#include <mutex>
#include <string>

#include <curl/curl.h>

namespace ravbot {

namespace {

void ensure_curl_global_init() {
  static std::once_flag curl_init_once;
  std::call_once(curl_init_once, [] {
    CURLcode result = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (result != CURLE_OK) {
      throw std::runtime_error("Failed to initialize libcurl: " +
                               std::string(curl_easy_strerror(result)));
    }
  });
}

}  // namespace

// --- CurlHandle ---

CurlHandle::CurlHandle() : handle_(nullptr) {
  ensure_curl_global_init();
  handle_ = curl_easy_init();
  if (!handle_) {
    throw std::runtime_error("Failed to initialize CURL");
  }
  curl_easy_setopt(handle_, CURLOPT_NOSIGNAL, 1L);
}

CurlHandle::~CurlHandle() {
  if (handle_) {
    curl_easy_cleanup(handle_);
  }
}

CurlHandle::CurlHandle(CurlHandle&& other) noexcept : handle_(other.handle_) {
  other.handle_ = nullptr;
}

CurlHandle& CurlHandle::operator=(CurlHandle&& other) noexcept {
  if (this != &other) {
    if (handle_) {
      curl_easy_cleanup(handle_);
    }
    handle_ = other.handle_;
    other.handle_ = nullptr;
  }
  return *this;
}

// --- CurlSlist ---

CurlSlist::~CurlSlist() {
  if (list_) {
    curl_slist_free_all(list_);
  }
}

CurlSlist::CurlSlist(CurlSlist&& other) noexcept : list_(other.list_) {
  other.list_ = nullptr;
}

CurlSlist& CurlSlist::operator=(CurlSlist&& other) noexcept {
  if (this != &other) {
    if (list_) {
      curl_slist_free_all(list_);
    }
    list_ = other.list_;
    other.list_ = nullptr;
  }
  return *this;
}

void CurlSlist::append(const char* str) {
  list_ = curl_slist_append(list_, str);
}

}  // namespace ravbot
