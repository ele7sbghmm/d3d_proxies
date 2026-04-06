#include "pch.h"

#include "context.h"
#include "mixins.h"

auto Buffer::Initialize() -> void {
	m_vertices.reserve(MAX_VERTICES);

	g_device->CreateVertexBuffer(
		MAX_VERTICES_LENGTH,
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		XYZNormalColor::FVF,
		D3DPOOL_DEFAULT,
		&m_vertexBuffer
	);

	m_initialized = true;
}

auto Buffer::Flush() -> void {
	m_length = m_vertices.size() * XYZNormalColor::STRIDE;
	m_primitiveCount = m_vertices.size() / 3;

	XYZNormalColor* dummy = nullptr;
	HRESULT hr = m_vertexBuffer->Lock(0, MAX_VERTICES_LENGTH, (BYTE**)&dummy, D3DLOCK_DISCARD);
	if (SUCCEEDED(hr)) {
		memcpy(dummy, m_vertices.data(), m_length);
		m_vertexBuffer->Unlock();
	}
}

auto Buffer::Clear() -> void {
	m_vertices.clear();
	m_length = 0;
	m_primitiveCount = 0;
}

auto Buffer::Draw() const -> void {
	if (m_primitiveCount == 0) return;

	g_device->SetVertexShader(XYZNormalColor::FVF);
	g_device->SetStreamSource(0, m_vertexBuffer, XYZNormalColor::STRIDE);
	g_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_primitiveCount);

	if (!m_vertexBuffer) throw;
}

auto RenderContext::Initialize() -> void {
	m_buffer_static.Initialize();
	m_buffer_dynamic.Initialize();
	m_buffer_transient.Initialize();
}

auto RenderContext::PopulateTransient() -> void {
	if (!m_buffer_transient.m_initialized)
		m_buffer_transient.Initialize();
	m_buffer_transient.Clear();

	mod::RenderManager* rm = mod::RenderManager::Get();
	if (!rm) return;
	mod::WorldRenderLayer* wrl = rm->GetWorldRenderLayer();
	Shar::SwapArray<Shar::DynaLoadListDSG> sll = wrl->mStaticLoadLists;
	Shar::DynaLoadListDSG& dll = sll.mpData[0];
	//mod::FenceArray& fences = *reinterpret_cast<mod::FenceArray*>(&dll.mFenceElems);
	//fences.AddVertices(m_buffer_transient.m_vertices);

	D3DXVECTOR3 position = *mod::Character::GetInstance()->GetPosition();
	float radius = 5.f;
	m_fences2d.BakeArray(dll.mFenceElems, position, radius);

	//m_buffer_transient.Flush();
}

auto RenderContext::Draw() const -> void {
	DWORD originalAlphaBlendEnable;
	g_device->GetRenderState(D3DRS_ALPHABLENDENABLE, &originalAlphaBlendEnable);

	g_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

	g_device->SetTexture(0, NULL);
	m_buffer_transient.Draw();

	g_device->SetRenderState(D3DRS_ALPHABLENDENABLE, originalAlphaBlendEnable);
}
