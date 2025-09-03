#include "TutorialApp.h"
#include "../Common/Helper.h"

bool TutorialApp::OnInitialize()
{
	if(!InitD3D())
		return false;
	
	if (!InitImGUI())
		return false;

	return true;
}

void TutorialApp::OnUninitialize()
{	
	UninitImGUI();
	UninitD3D();
}
void TutorialApp::OnUpdate()
{

}

void TutorialApp::OnRender()
{
	const float clear_color_with_alpha[4] = { m_ClearColor.x , m_ClearColor.y , m_ClearColor.z, m_ClearColor.w };	
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, clear_color_with_alpha);
	/////
	// �̰��� ���� ������ �մϴ�.
	// DeviceContext::Draw(); 
	/////

	/////////////////
	//�Ʒ����ʹ� ImGUI
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();



	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (m_show_demo_window)
		ImGui::ShowDemoWindow(&m_show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
	{	

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &m_show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &m_show_another_window);

		ImGui::SliderFloat("float", &m_f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		
		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			m_counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", m_counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

		std::string str;
		ImGui::Text("\nDisplay Memory");
		GetDisplayMemoryInfo(str);
		ImGui::Text("%s", str.c_str());
		ImGui::Text("\nProcess Memory");
		GetVirtualMemoryInfo(str);
		ImGui::Text("%s", str.c_str());

		ImGui::ColorEdit3("clear color", (float*)&m_ClearColor); // Edit 3 floats representing a color	
		ImGui::End();
	}

	// 3. Show another simple window.
	if (m_show_another_window)
	{
		ImGui::Begin("Another Window", &m_show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			m_show_another_window = false;
		ImGui::End();
	}
	ImGui::Render();	
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// ����ü�� ��ü.
	m_pSwapChain->Present(0, 0);
}

bool TutorialApp::InitD3D()
{
	// �����.
	HRESULT hr=0;

	// Create DXGI factory
	HR_T(CreateDXGIFactory1(__uuidof(IDXGIFactory4), (void**)m_pDXGIFactory.GetAddressOf()));
	HR_T(m_pDXGIFactory->EnumAdapters(0, reinterpret_cast<IDXGIAdapter**>(m_pDXGIAdapter.GetAddressOf())));

	// ����ü�� �Ӽ� ���� ����ü ����.
	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 1;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = m_hWnd;	// ����ü�� ����� â �ڵ� ��.
	swapDesc.Windowed = true;		// â ��� ���� ����.
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	// �����(�ؽ�ó)�� ����/���� ũ�� ����.
	swapDesc.BufferDesc.Width = m_ClientWidth;
	swapDesc.BufferDesc.Height = m_ClientHeight;
	// ȭ�� �ֻ��� ����.
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	// ���ø� ���� ����.
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	UINT creationFlags = 0;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	// 1. ��ġ ����.   2.����ü�� ����. 3.��ġ ���ؽ�Ʈ ����.
	HR_T(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, NULL, NULL,
		D3D11_SDK_VERSION, &swapDesc, &m_pSwapChain, &m_pDevice, NULL, &m_pDeviceContext));

	// 4. ����Ÿ�ٺ� ����.  (����۸� �̿��ϴ� ����Ÿ�ٺ�)	
	ID3D11Texture2D* pBackBufferTexture = nullptr;
	HR_T(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture));
	HR_T(m_pDevice->CreateRenderTargetView(pBackBufferTexture, NULL, &m_pRenderTargetView));  // �ؽ�ó�� ���� ���� ����
	SAFE_RELEASE(pBackBufferTexture);	//�ܺ� ���� ī��Ʈ�� ���ҽ�Ų��.
	// ���� Ÿ���� ���� ��� ���������ο� ���ε��մϴ�.
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL);	
	return true;
}

void TutorialApp::UninitD3D()
{
	SAFE_RELEASE(m_pRenderTargetView);	
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pDeviceContext);
	SAFE_RELEASE(m_pDevice);
}

bool TutorialApp::InitImGUI()
{
	/*
		ImGui �ʱ�ȭ.
	*/
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();


	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX11_Init(this->m_pDevice, this->m_pDeviceContext);

	//
	return true;
}

void TutorialApp::UninitImGUI()
{
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void TutorialApp::GetDisplayMemoryInfo(std::string& out)
{	
	DXGI_ADAPTER_DESC desc;
	m_pDXGIAdapter->GetDesc(&desc);

	DXGI_QUERY_VIDEO_MEMORY_INFO local,nonLocal;
	m_pDXGIAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &local);
	m_pDXGIAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &nonLocal);

	out =  std::to_string( (desc.DedicatedVideoMemory+ desc.SharedSystemMemory) / 1024 / 1024) + " MB\n";
	out += "Dedicated Video Memory : " + std::to_string(desc.DedicatedVideoMemory / 1024 / 1024) + " MB\n";	
	out += "Shared System Memory : " + std::to_string(desc.SharedSystemMemory/ 1024 / 1024) + " MB\n";
	
	out += "Local Video Memory: ";
	out += std::to_string(local.Budget / 1024 / 1024) + "MB" + " / " + std::to_string(local.CurrentUsage / 1024 / 1024) + " MB\n";
	out += "NonLocal Video Memory: ";
	out += std::to_string(nonLocal.Budget / 1024 / 1024) + "MB" + " / " + std::to_string(nonLocal.CurrentUsage / 1024 / 1024) + " MB";
}

void TutorialApp::GetVirtualMemoryInfo(std::string& out)
{	
	HANDLE hProcess = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS_EX pmc;
	pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS_EX);
	GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	out = "PrivateUsage: " + std::to_string((pmc.PrivateUsage) / 1024 / 1024) + " MB\n";
	out += "WorkingSetSize: " + std::to_string((pmc.WorkingSetSize) / 1024 / 1024) + " MB\n";
	out += "PagefileUsage: " + std::to_string((pmc.PagefileUsage) / 1024 / 1024) + " MB";
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK TutorialApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;

    return __super::WndProc(hWnd, message, wParam, lParam);
}
