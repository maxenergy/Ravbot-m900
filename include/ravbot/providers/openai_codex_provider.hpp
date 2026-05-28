// Copyright 2025 RavBot Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <memory>
#include <string>

#include <spdlog/spdlog.h>

#include "ravbot/auth/openai_codex_auth.hpp"
#include "ravbot/providers/curl_raii.hpp"
#include "ravbot/providers/llm_provider.hpp"

namespace ravbot {

class OpenAICodexProvider : public LLMProvider {
 public:
  OpenAICodexProvider(const std::string& base_url, int timeout,
                      std::shared_ptr<spdlog::logger> logger,
                      std::shared_ptr<auth::BearerTokenSource> token_source);

  ChatCompletionResponse
  ChatCompletion(const ChatCompletionRequest& request) override;
  void ChatCompletionStream(
      const ChatCompletionRequest& request,
      std::function<void(const ChatCompletionResponse&)> callback) override;
  std::string GetProviderName() const override;
  std::vector<std::string> GetSupportedModels() const override;

 private:
  std::string ResolveEndpoint() const;
  CurlSlist CreateHeaders(const std::string& bearer_token) const;

  std::string base_url_;
  int timeout_;
  std::shared_ptr<spdlog::logger> logger_;
  std::shared_ptr<auth::BearerTokenSource> token_source_;
};

}  // namespace ravbot
