// Released under the MIT License. See LICENSE for details.

#include "ballistica/ui/widget/check_box_widget.h"

#include "ballistica/audio/audio.h"
#include "ballistica/game/game.h"
#include "ballistica/graphics/component/empty_component.h"
#include "ballistica/graphics/component/simple_component.h"
#include "ballistica/python/python_context_call.h"
#include "ballistica/python/python_sys.h"
#include "ballistica/ui/ui.h"

namespace ballistica {

CheckBoxWidget::CheckBoxWidget() {
  SetText("CheckBox");
  text_.set_owner_widget(this);
  text_.set_valign(TextWidget::VAlign::kCenter);
  text_.set_halign(TextWidget::HAlign::kLeft);
}

CheckBoxWidget::~CheckBoxWidget() = default;

void CheckBoxWidget::SetOnValueChangeCall(PyObject* call_tuple) {
  on_value_change_call_ = Object::New<PythonContextCall>(call_tuple);
}

void CheckBoxWidget::SetText(const std::string& text) {
  text_.SetText(text);
  have_text_ = (!text.empty());
}

void CheckBoxWidget::SetWidth(float width_in) {
  highlight_dirty_ = box_dirty_ = check_dirty_ = true;
  width_ = width_in;
  text_.SetWidth(width_in - (2 * box_padding_ + box_size_ + 4));
}

void CheckBoxWidget::SetHeight(float height_in) {
  highlight_dirty_ = box_dirty_ = check_dirty_ = true;
  height_ = height_in;
  text_.SetHeight(height_in);
}

void CheckBoxWidget::Draw(RenderPass* pass, bool draw_transparent) {
  millisecs_t real_time = GetRealTime();

  have_drawn_ = true;
  float l = 0.0f;
  float r = l + width_;
  float b = 0.0f;
  float t = b + height_;

  Vector3f tilt = 0.01f * g_graphics->tilt();
  if (draw_control_parent()) {
    tilt += 0.02f * g_graphics->tilt();
  }
  float extra_offs_x = -tilt.y;
  float extra_offs_y = tilt.x;

  if (have_text_ && draw_transparent
      && ((selected() && g_ui->ShouldHighlightWidgets())
          || (pressed_ && mouse_over_))) {
    // Draw glow (at depth 0.9f).
    float m;
    if (pressed_ && mouse_over_) {
      m = 2.0f;
    } else if (IsHierarchySelected()) {
      m = 0.5f
          + std::abs(sinf(static_cast<float>(real_time) * 0.006467f) * 0.4f);
    } else {
      m = 0.25f;
    }

    if (highlight_dirty_) {
      float l_border, r_border, b_border, t_border;
      l_border = 10.0f;
      r_border = 0.0f;
      b_border = 11.0f;
      t_border = 11.0f;
      highlight_width_ = r - l + l_border + r_border;
      highlight_height_ = t - b + b_border + t_border;
      highlight_center_x_ = l - l_border + highlight_width_ * 0.5f;
      highlight_center_y_ = b - b_border + highlight_height_ * 0.5f;
      highlight_dirty_ = false;
    }
    SimpleComponent c(pass);
    c.SetTransparent(true);
    c.SetPremultiplied(true);
    c.SetColor(0.25f * m, 0.3f * m, 0, 0.3f * m);
    c.SetTexture(g_media->GetTexture(SystemTextureID::kGlow));
    c.PushTransform();
    c.Translate(highlight_center_x_, highlight_center_y_);
    c.Scale(highlight_width_, highlight_height_);
    c.DrawModel(g_media->GetModel(SystemModelID::kImage4x1));
    c.PopTransform();
    c.Submit();
  }

  float glow_amt = 1.0f;

  {
    float box_l = l + box_padding_;
    float box_r = box_l + box_size_;
    float box_b = b + (t - b) / 2 - box_size_ / 2;
    float box_t = box_b + box_size_;

    if (pressed_ && mouse_over_) {
      glow_amt = 2.0f;
    } else if (IsHierarchySelected() && g_ui->ShouldHighlightWidgets()) {
      glow_amt =
          0.8f
          + std::abs(sinf(static_cast<float>(real_time) * 0.006467f) * 0.3f);
    }

    // Button portion (depth 0.1f-0.5f).
    {
      if (box_dirty_) {
        float l_border, r_border, b_border, t_border;
        l_border = 8;
        r_border = 12;
        b_border = 6;
        t_border = 6;
        box_width_ = box_r - box_l + l_border + r_border;
        box_height_ = box_t - box_b + b_border + t_border;
        box_center_x_ = box_l - l_border + box_width_ * 0.5f;
        box_center_y_ = box_b - b_border + box_height_ * 0.5f;
        box_dirty_ = false;
      }

      SimpleComponent c(pass);
      c.SetTransparent(draw_transparent);
      c.SetColor(glow_amt * color_r_, glow_amt * color_g_, glow_amt * color_b_,
                 1);
      c.SetTexture(g_media->GetTexture(SystemTextureID::kUIAtlas));
      c.PushTransform();
      c.Translate(box_center_x_ + extra_offs_x, box_center_y_ + extra_offs_y,
                  0.1f);
      c.Scale(box_width_, box_height_, 0.4f);
      c.DrawModel(g_media->GetModel(draw_transparent
                                        ? SystemModelID::kButtonSmallTransparent
                                        : SystemModelID::kButtonSmallOpaque));
      c.PopTransform();
      c.Submit();
    }

    // Check portion.
    if (draw_transparent) {
      if (check_dirty_) {
        float s = 1;
        if (real_time - last_change_time_ < 100) {
          s = static_cast<float>(real_time - last_change_time_) / 100;
        }
        if (!checked_) s = 1.0f - s;

        float check_offset_h = -2;
        float check_offset_v = -2;

        check_width_ = 45 * s;
        check_height_ = 45 * s;
        check_center_x_ =
            box_l + 11 - 18 * s + check_offset_h + check_width_ * 0.5f;
        check_center_y_ =
            box_b + 10 - 18 * s + check_offset_v + check_height_ * 0.5f;

        // Only set clean once our transition is over.
        if (real_time - last_change_time_ > 100) check_dirty_ = false;
      }

      // Draw check in z depth from 0.5f to 1.
      SimpleComponent c(pass);
      c.SetTransparent(draw_transparent);
      if (is_radio_button_) {
        c.SetTexture(g_media->GetTexture(SystemTextureID::kNub));
      } else {
        c.SetTexture(g_media->GetTexture(SystemTextureID::kUIAtlas));
      }

      if (mouse_over_ && g_platform->IsRunningOnDesktop()) {
        c.SetColor(1.0f * glow_amt, 0.7f * glow_amt, 0, 1);
      } else {
        c.SetColor(1.0f * glow_amt, 0.6f * glow_amt, 0, 1);
      }
      c.PushTransform();

      if (is_radio_button_) {
        c.Translate(check_center_x_ + 1 + 3.0f * extra_offs_x,
                    check_center_y_ + 2 + 3.0f * extra_offs_y, 0.5f);
        c.Scale(check_width_ * 0.45f, check_height_ * 0.45f, 0.5f);
        c.Translate(-0.17f, -0.17f, 0.5f);
        c.DrawModel(g_media->GetModel(SystemModelID::kImage1x1));
      } else {
        c.Translate(check_center_x_ + 3.0f * extra_offs_x,
                    check_center_y_ + 3.0f * extra_offs_y, 0.5f);
        c.Scale(check_width_, check_height_, 0.5f);
        c.DrawModel(g_media->GetModel(SystemModelID::kCheckTransparent));
      }
      c.PopTransform();
      c.Submit();
    }
  }

  // Draw our text in z depth 0.5f to 1.
  EmptyComponent c(pass);
  c.SetTransparent(draw_transparent);
  c.PushTransform();
  c.Translate(2 * box_padding_ + box_size_ + 10, 0, 0.5f);
  c.Scale(1, 1, 0.5f);
  c.Submit();
  float cs = glow_amt;
  text_.set_color(cs * text_color_r_, cs * text_color_g_, cs * text_color_b_,
                  text_color_a_);
  text_.Draw(pass, draw_transparent);
  c.PopTransform();
  c.Submit();
}

// for our center we return something near center of the checkbox; not our text
void CheckBoxWidget::GetCenter(float* x, float* y) {
  *x = tx() + scale() * GetWidth() * 0.2f;
  *y = ty() + scale() * GetHeight() * 0.5f;
}

void CheckBoxWidget::SetValue(bool value) {
  if (value == checked_) {
    return;
  }
  check_dirty_ = true;

  // Don't animate if we're setting initial values.
  if (checked_ != value && have_drawn_) {
    last_change_time_ = GetRealTime();
  }
  checked_ = value;
}

void CheckBoxWidget::Activate() {
  g_audio->PlaySound(g_media->GetSound(SystemSoundID::kSwish3));
  checked_ = !checked_;
  check_dirty_ = true;
  last_change_time_ = GetRealTime();
  if (on_value_change_call_.exists()) {
    PythonRef args(Py_BuildValue("(O)", checked_ ? Py_True : Py_False),
                   PythonRef::kSteal);

    // Call this in the next cycle (don't want to risk mucking with UI from
    // within a UI loop)
    g_game->PushPythonWeakCallArgs(
        Object::WeakRef<PythonContextCall>(on_value_change_call_), args);
  }
}

auto CheckBoxWidget::HandleMessage(const WidgetMessage& m) -> bool {
  // How far outside button touches register.
  float left_overlap, top_overlap, right_overlap, bottom_overlap;
  if (g_platform->IsRunningOnDesktop()) {
    left_overlap = 3.0f;
    top_overlap = 1.0f;
    right_overlap = 0.0f;
    bottom_overlap = 0.0f;
  } else {
    left_overlap = 12.0f;
    top_overlap = 10.0f;
    right_overlap = 13.0f;
    bottom_overlap = 15.0f;
  }

  switch (m.type) {
    case WidgetMessage::Type::kMouseMove: {
      float x = m.fval1;
      float y = m.fval2;
      bool claimed = (m.fval3 > 0.0f);
      if (claimed)
        mouse_over_ = false;
      else
        mouse_over_ =
            ((x >= (-left_overlap)) && (x < (width_ + right_overlap))
             && (y >= (-bottom_overlap)) && (y < (height_ + top_overlap)));
      return mouse_over_;
    }
    case WidgetMessage::Type::kMouseDown: {
      float x = m.fval1;
      float y = m.fval2;
      if ((x >= (-left_overlap)) && (x < (width_ + right_overlap))
          && (y >= (-bottom_overlap)) && (y < (height_ + top_overlap))) {
        GlobalSelect();
        pressed_ = true;
        return true;
      } else {
        return false;
      }
    }
    case WidgetMessage::Type::kMouseUp: {
      float x = m.fval1;
      float y = m.fval2;
      bool claimed = (m.fval3 > 0.0f);

      // Radio-style boxes can't be un-checked.
      if (pressed_) {
        pressed_ = false;

        // if they're still over us and unclaimed, toggle.
        if ((x >= (-left_overlap)) && (x < (width_ + right_overlap))
            && (y >= (-bottom_overlap)) && (y < (height_ + top_overlap))
            && !claimed) {
          // Radio-style buttons don't allow unchecking.
          if (!is_radio_button_ || !checked_) {
            Activate();
          }
        }
        return true;  // If we're pressed, claim any mouse-ups presented to us.
      }
      break;
    }
    default:
      break;
  }
  return false;
}

void CheckBoxWidget::OnLanguageChange() { text_.OnLanguageChange(); }

}  // namespace ballistica
