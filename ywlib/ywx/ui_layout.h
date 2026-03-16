#pragma once
#include "ywx/window.h"

namespace yw::ui {

class layout : public control {
public:
  class slot : public control::slot {
  public:
    window::slotid window_id{};
    std::vector<slotid> children{};

    virtual ~slot() noexcept {
      for (auto cid : children) system::controls.erase(cid);
      if (const auto lsp = _slot_address<slot>(layout_id)) lsp->detach(id);
    }

    virtual tuple<float2, uint2> minimum_size() const noexcept override {
      tuple<float2, uint2> result{};
      for (const auto& cid : children)
        if (const auto csp = system::controls.get(cid)) {
          const auto [ms, uc] = csp->minimum_size();
          result.first.x = yw::max(result.first.x, ms.x);
          result.first.y += yw::max(ms.y, 0.0f);
          result.second.x |= uc.x;
          result.second.y += uc.y;
        }
      return result;
    }

    virtual bool attach(const slotid& ChildId) override {
      children.push_back(ChildId);
      return true;
    }

    virtual void detach(const slotid& ChildId) override { std::erase(children, ChildId); }

    virtual void make_dirty() noexcept override {
      if (const auto wsp = system::windows.get(window_id)) wsp->dirty = true;
    }

    virtual void make_mess() noexcept override {
      if (const auto wsp = system::windows.get(window_id)) wsp->messy = true;
    }

    virtual slotid hit_test(float2 Pt) const noexcept override {

    }


  };
};
}
