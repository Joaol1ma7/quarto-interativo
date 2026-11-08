#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <wincodec.h>
#include "image_loader.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
template <typename T> struct ComObject {
    T* valor = nullptr;
    ~ComObject() { if (valor) valor->Release(); }
    T** out() { return &valor; }
    T* operator->() const { return valor; }
};

struct ComApartment {
    bool inicializado = false;
    ComApartment() {
        const HRESULT resultado = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (SUCCEEDED(resultado)) inicializado = true;
        else if (resultado != RPC_E_CHANGED_MODE)
            throw std::runtime_error("Falha ao iniciar o carregamento da imagem do quadro.");
    }
    ~ComApartment() { if (inicializado) CoUninitialize(); }
};

void require(HRESULT resultado, const char* mensagem) {
    if (FAILED(resultado)) throw std::runtime_error(mensagem);
}

std::wstring artworkPath() {
    std::vector<wchar_t> buffer(32768);
    const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0 || length >= buffer.size())
        throw std::runtime_error("Nao foi possivel localizar o executavel.");
    std::wstring caminho(buffer.data(), length);
    const auto separador = caminho.find_last_of(L"\\/");
    return caminho.substr(0, separador + 1) + L"assets\\quadro.jpg";
}
}

ImageData loadArtworkImage() {
    ComApartment apartamento;
    ComObject<IWICImagingFactory> fabrica;
    require(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                             IID_IWICImagingFactory,
                             reinterpret_cast<void**>(fabrica.out())),
            "Falha ao iniciar o decodificador de imagem.");
    ComObject<IWICBitmapDecoder> decodificador;
    const std::wstring caminho = artworkPath();
    require(fabrica->CreateDecoderFromFilename(caminho.c_str(), nullptr, GENERIC_READ,
                                                WICDecodeMetadataCacheOnLoad, decodificador.out()),
            "Nao foi possivel abrir assets/quadro.jpg ao lado do executavel.");
    ComObject<IWICBitmapFrameDecode> quadro;
    require(decodificador->GetFrame(0, quadro.out()), "Falha ao ler a fotografia do quadro.");
    ComObject<IWICFormatConverter> conversor;
    require(fabrica->CreateFormatConverter(conversor.out()), "Falha ao conversor a fotografia.");
    require(conversor->Initialize(quadro.valor, GUID_WICPixelFormat24bppRGB,
                                  WICBitmapDitherTypeNone, nullptr, 0.0,
                                  WICBitmapPaletteTypeCustom),
            "Formato da fotografia do quadro nao suportado.");
    UINT largura = 0, altura = 0;
    require(conversor->GetSize(&largura, &altura), "Falha ao obter tamanho da fotografia.");
    if (largura == 0 || altura == 0 || largura > 16384 || altura > 16384)
        throw std::runtime_error("Tamanho invalido da fotografia do quadro.");
    const UINT passoLinha = largura * 3;
    std::vector<std::uint8_t> pixels(static_cast<std::size_t>(passoLinha) * altura);
    require(conversor->CopyPixels(nullptr, passoLinha, static_cast<UINT>(pixels.size()), pixels.data()),
            "Falha ao copiar os pixels da fotografia.");
    // WIC entrega a primeira linha no topo; a coordenada V=0 do OpenGL fica embaixo.
    for (UINT y = 0; y < altura / 2; ++y)
        std::swap_ranges(pixels.begin() + static_cast<std::size_t>(y) * passoLinha,
                         pixels.begin() + static_cast<std::size_t>(y + 1) * passoLinha,
                         pixels.begin() + static_cast<std::size_t>(altura - 1 - y) * passoLinha);
    return {static_cast<int>(largura), static_cast<int>(altura), std::move(pixels)};
}
