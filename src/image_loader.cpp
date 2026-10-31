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
    T* value = nullptr;
    ~ComObject() { if (value) value->Release(); }
    T** out() { return &value; }
    T* operator->() const { return value; }
};

struct ComApartment {
    bool initialized = false;
    ComApartment() {
        const HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (SUCCEEDED(result)) initialized = true;
        else if (result != RPC_E_CHANGED_MODE)
            throw std::runtime_error("Falha ao iniciar o carregamento da imagem do quadro.");
    }
    ~ComApartment() { if (initialized) CoUninitialize(); }
};

void require(HRESULT result, const char* message) {
    if (FAILED(result)) throw std::runtime_error(message);
}

std::wstring artworkPath() {
    std::vector<wchar_t> buffer(32768);
    const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0 || length >= buffer.size())
        throw std::runtime_error("Nao foi possivel localizar o executavel.");
    std::wstring path(buffer.data(), length);
    const auto separator = path.find_last_of(L"\\/");
    return path.substr(0, separator + 1) + L"assets\\quadro.jpg";
}
}

ImageData loadArtworkImage() {
    ComApartment apartment;
    ComObject<IWICImagingFactory> factory;
    require(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                             IID_IWICImagingFactory,
                             reinterpret_cast<void**>(factory.out())),
            "Falha ao iniciar o decodificador de imagem.");
    ComObject<IWICBitmapDecoder> decoder;
    const std::wstring path = artworkPath();
    require(factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ,
                                                WICDecodeMetadataCacheOnLoad, decoder.out()),
            "Nao foi possivel abrir assets/quadro.jpg ao lado do executavel.");
    ComObject<IWICBitmapFrameDecode> frame;
    require(decoder->GetFrame(0, frame.out()), "Falha ao ler a fotografia do quadro.");
    ComObject<IWICFormatConverter> converter;
    require(factory->CreateFormatConverter(converter.out()), "Falha ao converter a fotografia.");
    require(converter->Initialize(frame.value, GUID_WICPixelFormat24bppRGB,
                                  WICBitmapDitherTypeNone, nullptr, 0.0,
                                  WICBitmapPaletteTypeCustom),
            "Formato da fotografia do quadro nao suportado.");
    UINT width = 0, height = 0;
    require(converter->GetSize(&width, &height), "Falha ao obter tamanho da fotografia.");
    if (width == 0 || height == 0 || width > 16384 || height > 16384)
        throw std::runtime_error("Tamanho invalido da fotografia do quadro.");
    const UINT stride = width * 3;
    std::vector<std::uint8_t> pixels(static_cast<std::size_t>(stride) * height);
    require(converter->CopyPixels(nullptr, stride, static_cast<UINT>(pixels.size()), pixels.data()),
            "Falha ao copiar os pixels da fotografia.");
    // WIC entrega a primeira linha no topo; a coordenada V=0 do OpenGL fica embaixo.
    for (UINT y = 0; y < height / 2; ++y)
        std::swap_ranges(pixels.begin() + static_cast<std::size_t>(y) * stride,
                         pixels.begin() + static_cast<std::size_t>(y + 1) * stride,
                         pixels.begin() + static_cast<std::size_t>(height - 1 - y) * stride);
    return {static_cast<int>(width), static_cast<int>(height), std::move(pixels)};
}
