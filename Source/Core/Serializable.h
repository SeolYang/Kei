#pragma once

namespace sy
{
class Serializable
{
public:
    virtual ~Serializable() = default;

	virtual json Serialize() const = 0;
    virtual void Deserialize(const json& root) = 0;
};
} // namespace sy