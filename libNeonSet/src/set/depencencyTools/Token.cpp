#include <utility>

#include "Neon/set//dependency/Token.h"

namespace Neon::set::dataDependency {

Token::Token(MultiXpuDataUid uid,
             AccessType      access,
             Compute         compute)
{
    update(uid, access, compute);
}

auto Token::update(MultiXpuDataUid uid,
                   AccessType      access,
                   Compute         compute) -> void
{
    mUid = uid;
    mAccess = access;
    mCompute = compute;
    setHaloUpdate(
        [&](Neon::set::HuOptions&) -> void {
        NeonException exp("Token");
        exp << "This is not a stencil token and this is not a valid operation";
        NEON_THROW(exp); },
        [&](Neon::SetIdx, Neon::set::HuOptions&) -> void {
            NeonException exp("Token");
            exp << "This is not a stencil token and this is not a valid operation";
            NEON_THROW(exp);
        });
}

auto Token::access() const -> AccessType
{
    return mAccess;
}

auto Token::compute() const -> Compute
{
    return mCompute;
}

auto Token::uid() const -> MultiXpuDataUid
{
    return mUid;
}

auto Token::toString() const -> std::string
{
    return " uid " + std::to_string(mUid) +
           " [Op " + AccessTypeUtils::toString(mAccess) +
           " Model " + Neon::ComputeUtils::toString(mCompute) + "]";
}


auto Token::
    setHaloUpdate(std::function<void(Neon::set::HuOptions& opt)>               hu,
                  std::function<void(Neon::SetIdx, Neon::set::HuOptions& opt)> huPerDevice) -> void
{
    mHu = std::move(hu);
    mHuPerDevice = std::move(huPerDevice);
}

auto Token::getHaloUpdate() const
    -> const std::function<void(Neon::set::HuOptions& opt)>&
{
    return mHu;
}

auto Token::getHaloUpdatePerDevice() const
    -> const std::function<void(Neon::SetIdx, Neon::set::HuOptions& opt)>&
{
    return mHuPerDevice;
}
auto Token::mergeAccess(AccessType tomerge) -> void
{
    mAccess = AccessTypeUtils::merge(mAccess, tomerge);
}

}  // namespace Neon::set::dataDependency