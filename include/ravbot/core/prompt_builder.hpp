// Copyright 2025 RavBot Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <memory>
#include <string>

#include <spdlog/spdlog.h>

namespace ravbot {

class MemoryManager;
class SkillLoader;
class ToolRegistry;

struct AgentConfig;
struct RavBotConfig;

class PromptBuilder {
 public:
  PromptBuilder(std::shared_ptr<MemoryManager> memory_manager,
                std::shared_ptr<SkillLoader> skill_loader,
                std::shared_ptr<ToolRegistry> tool_registry,
                const RavBotConfig* config = nullptr);

  // Full system prompt: SOUL + AGENTS + TOOLS + skills + memory + runtime info
  std::string BuildFull(const std::string& agent_id = "default") const;

  // Minimal system prompt: identity + tools only
  std::string BuildMinimal(const std::string& agent_id = "default") const;

 private:
  std::shared_ptr<MemoryManager> memory_manager_;
  std::shared_ptr<SkillLoader> skill_loader_;
  std::shared_ptr<ToolRegistry> tool_registry_;
  const RavBotConfig* config_ = nullptr;

  std::string get_section(const std::string& filename) const;
  std::string get_runtime_info() const;
};

}  // namespace ravbot
