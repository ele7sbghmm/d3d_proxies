#include "pch.h"

#include "gui.h"
#include "vertex.h"
#include <d3dx8.h>
#include "mixins.h"

struct Points { float xs[5]; float ys[5]; };
auto Pointer(const float scale, const float ox, const float oy) -> Points {
	Points p{};
	float x{ 0.f }, y{ -.5f };

	p.xs[0] = x * scale + ox; p.ys[0] = y * scale + oy;
	p.xs[4] = x * scale + ox; p.ys[4] = y * scale + oy;

	x = sinf(.35f); y = cosf(.35f);
	p.xs[1] = -x * scale + ox; p.ys[1] = y * scale + oy;

	x = 0.f; y = .25f;
	p.xs[2] = x * scale + ox; p.ys[2] = y * scale + oy;

	x = sinf(.35f); y = cosf(.35f);
	p.xs[3] = x * scale + ox; p.ys[3] = y * scale + oy;

	return p;
}


auto Render3DGui() -> void {
	//Buffer& buffer = g_context.m_buffer_transient;
	FenceRenderer2d& fences = g_context.m_fences2d;
	if (!fences.m_inactive.m_x.size()) return;

	ImGui_ImplDX8_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuiIO& io = ImGui::GetIO();
	io.MouseDrawCursor = true;

	ImPlotSpec spec_inactive;
	ImPlotSpec spec_active;
	ImPlotSpec spec_overflow;
	spec_inactive.LineColor = ImVec4(1.f, 1.f, 1.f, 1.f);
	spec_active.LineColor = ImVec4(1.f, 0.f, 0.f, 1.f);
	spec_overflow.LineColor = ImVec4(0.f, 1.f, 0.f, 1.f);
	spec_inactive.Flags = ImPlotLineFlags_Segments;
	spec_active.Flags = ImPlotLineFlags_Segments;
	spec_overflow.Flags = ImPlotLineFlags_Segments;
	const size_t overflow = 8;

	D3DXVECTOR3 position = *mod::Character::GetInstance()->GetPosition();
	float zoom = 20.f;
	ImPlot::SetNextAxesLimits(
		position.x - zoom, position.x + zoom,
		position.z - zoom, position.z + zoom,
		ImGuiCond_Always
	);

	if (ImGui::Begin("##MinimapBegin")) {
		if (ImPlot::BeginPlot("##MinimapPlot"), ImVec2(-1, 1), ImPlotFlags_Equal | ImPlotFlags_CanvasOnly) {
			ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoGridLines);
			ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoGridLines);

			Points pointer = Pointer(2.f, position.x, position.z);
			ImPlotSpec pointer_spec;
			pointer_spec.LineColor = ImVec4(1.f, 1.f, 1.f, 1.f);
			ImPlot::PlotLine("pointer", &pointer.xs[0], &pointer.ys[0], 5, pointer_spec);

			char legend_buffer[256];
			if (fences.m_inactive.m_x.size()) {
				sprintf_s(legend_buffer, "inactive %d###inactive", fences.m_inactive.m_x.size() / 2);
				ImPlot::PlotLine(legend_buffer,
					fences.m_inactive.m_x.data(),
					fences.m_inactive.m_y.data(),
					fences.m_inactive.m_x.size(),
					spec_inactive);
			}
			if (fences.m_active.m_x.size()) {
				sprintf_s(legend_buffer, "active %d###active", min(fences.m_active.m_x.size(), overflow * 2) / 2);
				ImPlot::PlotLine(legend_buffer,
					fences.m_active.m_x.data(),
					fences.m_active.m_y.data(),
					fences.m_active.m_x.size(),
					spec_active);
			}
			if (fences.m_active.m_x.size() > overflow * 2) {
				sprintf_s(legend_buffer, "overflow %d###overflow", (fences.m_active.m_x.size() - overflow * 2) / 2);
				ImPlot::PlotLine(legend_buffer,
					&fences.m_active.m_x[overflow * 2],
					&fences.m_active.m_y[overflow * 2],
					fences.m_active.m_x.size() - overflow * 2,
					spec_overflow);
			}

			ImPlot::EndPlot();
		}
		ImGui::End();
	}

	fences.Clear();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX8_RenderDrawData(ImGui::GetDrawData());
}
