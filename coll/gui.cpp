#include "pch.h"

#include "gui.h"
#include "mod.h"

auto Gui::begin(CollVisContext& ctx) {
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Once);
	if (ImGui::Begin("##begin", &g_collviscontext.m_show_gui)) {
		if (ImGui::BeginTabBar("##tabbar")) {
			if (ImGui::BeginTabItem("toggles##tab")) {
				ImGui::Checkbox("draw passive entities##fence", &ctx.m_draw_passive);
				ImGui::Checkbox("draw active entities##fence", &ctx.m_draw_active);
				ImGui::Checkbox("freeze position", &ctx.m_freeze_position);
				ImGui::Checkbox("freeze y", &ctx.m_freeze_y);

				ImGui::SeparatorText("fence");
				ImGui::Checkbox("draw", &ctx.m_fence.m_draw);
				ImGui::DragFloat("height", &ctx.m_fence.m_height, 1.f, 0.f, 100.f, "%.1f", ImGuiSliderFlags_Logarithmic);

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("colors##tab")) {
				ImGui::SeparatorText("material");
				float* emissive = &ctx.m_material.Emissive.r;
				float* ambient = &ctx.m_material.Ambient.r;
				if (ImGui::SliderFloat("ambient", ambient, 0.f, 1.f, "%.2f"))
					ctx.m_material.Ambient = { *ambient, *ambient, *ambient, *ambient };
				if (ImGui::SliderFloat("emissive", emissive, 0.f, 1.f, "%.2f"))
					ctx.m_material.Emissive = { *emissive, *emissive, *emissive, *emissive };

				ImGui::SeparatorText("fence");
				ImGui::ColorEdit4("passive##fence_color", (float*)&ctx.m_fence.m_color_passive);
				ImGui::ColorEdit4("active##fence_color", (float*)&ctx.m_fence.m_color_active);
				ImGui::ColorEdit4("collidable##fence_color", (float*)&ctx.m_fence.m_color_collidable);
				ImGui::ColorEdit4("overflow##fence_color", (float*)&ctx.m_fence.m_color_overflow);

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("states")) {
				RenderState& state = ctx.m_state;
				ImGui::Checkbox("LIGHTING", &state.lighting);
				ImGui::Checkbox("NORMALIZENORMALS", &state.normalize);
				ImGui::Checkbox("COLORVERTEX", &state.colorVertex);
				ImGui::Checkbox("ALPHABLENDENABLE", &state.alphaBlendEnable);
				ImGui::Checkbox("ZENABLE", &state.zEnable);
				ImGui::Checkbox("ZWRITEENABLE", &state.zWriteEnable);

				if (ImGui::BeginCombo("SRCBLEND", state.blend[state.srcBlend])) {
					for (std::size_t i = 0; i < IM_ARRAYSIZE(state.blend); i++) {
						const bool is_selected = (state.srcBlend == i);
						if (ImGui::Selectable(state.blend[i], is_selected)) state.srcBlend = i;
						if (is_selected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				if (ImGui::BeginCombo("DESTBLEND", state.blend[state.destBlend])) {
					for (std::size_t i = 0; i < IM_ARRAYSIZE(state.blend); i++) {
						const bool is_selected = (state.destBlend == i);
						if (ImGui::Selectable(state.blend[i], is_selected)) state.destBlend = i;
						if (is_selected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				if (ImGui::BeginCombo("CULLMODE", state.cullmode[state.cullMode])) {
					for (std::size_t i = 0; i < IM_ARRAYSIZE(state.cullmode); i++) {
						const bool is_selected = (state.cullMode == i);
						if (ImGui::Selectable(state.cullmode[i], is_selected)) state.cullMode = i;
						if (is_selected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}

auto Gui::new_frame(CollVisContext& ctx) -> void {
	if (!ctx.m_show_gui) return;

	ImGui_ImplDX8_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	
	ImGui::GetIO().MouseDrawCursor = true;

	begin(ctx);

	ImGui::Render();
	ImGui_ImplDX8_RenderDrawData(ImGui::GetDrawData());
}
