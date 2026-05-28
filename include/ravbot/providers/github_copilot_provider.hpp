// Copyright 2025 RavBot Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <memory>
#include <mutex>
#include <optional>

#include <spdlog/spdlog.h>

#include "ravbot/auth/github_copilot_auth.hpp"
#include "ravbot/providers/openai_provider.hpp"

namespace ravbot {

class GitHubCopilotProvider : public OpenAIProvider {
 public:
  GitHubCopilotProvider(
      int timeout, std::shared_ptr<spdlog::logger> logger,
      std::shared_ptr<auth::GitHubCopilotRuntimeResolverInterface> resolver);

  std::string GetProviderName() const override;
  std::vector<std::string> GetSupportedModels() const override;

 protected:
  std::string ResolveApiKey() const override;
  std::string ResolveBaseUrl() const override;
  std::string ProviderId() const override;
  CurlSlist CreateHeaders() const override;

 private:
  const auth::GitHubCopilotRuntimeCredential& ResolveRuntimeCredential() const;

  mutable std::mutex runtime_mu_;
  mutable std::optional<auth::GitHubCopilotRuntimeCredential> cached_runtime_;
  std::shared_ptr<auth::GitHubCopilotRuntimeResolverInterface> resolver_;
};

}  // namespace ravbot
