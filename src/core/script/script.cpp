#include "core/script/script.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <utility>

namespace ce::core
{

namespace
{

long to_step(const KvSection& section, double step_rate)
{
    const double at = section.require_number("at");
    if (at < 0.0)
    {
        throw KvError(section.find("at")->line, "'at' must not be negative");
    }
    return std::lround(at * step_rate);
}

GameButton button_of(const KvSection& section)
{
    const std::string name = section.require_text("button");
    const std::optional<GameButton> button = button_from_name(name);
    if (!button)
    {
        throw KvError(section.find("button")->line,
                      "unknown button '" + name + "', use a, b, x or y");
    }
    return *button;
}

bool valid_shot_name(const std::string& name)
{
    return std::all_of(name.begin(), name.end(),
                       [](unsigned char c)
                       {
                           return std::isalnum(c) != 0 || c == '_' || c == '-';
                       });
}

} // namespace

Script parse_script(const KvDocument& document, double step_rate)
{
    Script script;
    bool has_end = false;
    for (const KvSection& section : document.sections)
    {
        if (section.name().empty())
        {
            if (!section.entries().empty())
            {
                throw KvError(section.entries().front().line,
                              "put keys under [press], [release], [shot] or [end]");
            }
            continue;
        }
        ScriptAction action;
        action.step = to_step(section, step_rate);
        if (section.name() == "press" || section.name() == "release")
        {
            action.kind =
                section.name() == "press" ? ScriptAction::Kind::Press : ScriptAction::Kind::Release;
            action.button = button_of(section);
        }
        else if (section.name() == "shot")
        {
            action.kind = ScriptAction::Kind::Shot;
            action.name = section.require_text("name");
            if (!valid_shot_name(action.name))
            {
                throw KvError(section.find("name")->line,
                              "shot name may only have letters, digits, '_' and '-'");
            }
        }
        else if (section.name() == "end")
        {
            if (has_end)
            {
                throw KvError(section.line(), "only one [end] section is allowed");
            }
            has_end = true;
            script.end_step = action.step;
            continue;
        }
        else
        {
            throw KvError(section.line(), "unknown section [" + section.name() + "]");
        }
        script.actions.push_back(std::move(action));
    }
    if (!has_end)
    {
        throw KvError(0, "missing [end] section");
    }
    for (const ScriptAction& action : script.actions)
    {
        if (action.step > script.end_step)
        {
            throw KvError(0, "an action comes after [end]");
        }
    }
    std::stable_sort(script.actions.begin(), script.actions.end(),
                     [](const ScriptAction& a, const ScriptAction& b)
                     {
                         return a.step < b.step;
                     });
    return script;
}

Script load_script(const std::filesystem::path& path, double step_rate)
{
    try
    {
        return parse_script(load_kv_file(path), step_rate);
    }
    catch (const KvError& error)
    {
        if (!error.file().empty())
        {
            throw;
        }
        throw KvError(error.line(), error.detail(), path.filename().string());
    }
}

ScriptPlayer::ScriptPlayer(Script script)
    : script_(std::move(script))
{
}

bool ScriptPlayer::done() const
{
    return step_ > script_.end_step;
}

long ScriptPlayer::step() const
{
    return step_;
}

const ButtonsDown& ScriptPlayer::buttons()
{
    while (next_action_ < script_.actions.size() && script_.actions[next_action_].step <= step_)
    {
        const ScriptAction& action = script_.actions[next_action_];
        const auto index = static_cast<std::size_t>(action.button);
        if (action.kind == ScriptAction::Kind::Press)
        {
            down_[index] = true;
        }
        else if (action.kind == ScriptAction::Kind::Release)
        {
            down_[index] = false;
        }
        ++next_action_;
    }
    return down_;
}

std::vector<std::string> ScriptPlayer::shots() const
{
    std::vector<std::string> names;
    for (const ScriptAction& action : script_.actions)
    {
        if (action.kind == ScriptAction::Kind::Shot && action.step == step_)
        {
            names.push_back(action.name);
        }
    }
    return names;
}

void ScriptPlayer::next()
{
    ++step_;
}

} // namespace ce::core
