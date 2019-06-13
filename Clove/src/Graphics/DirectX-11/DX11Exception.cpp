#include "clvpch.hpp"
#include "DX11Exception.hpp"

#include <dxerr.h>

namespace clv::gfx{
	DX11Exception::~DX11Exception() = default;

	DX11Exception::DX11Exception(int lineNum, const char* file, HRESULT hr, const std::vector<std::string>& messages)
		: CloveException(lineNum, file)
		, hr(hr){
		for(const auto& message : messages){
			info += message;
			info.push_back('\n');
		}

		if(!info.empty()){
			info.pop_back();
		}
	}

	const char* DX11Exception::what() const noexcept{
		std::ostringstream oss;
		oss << getType() << std::endl
			<< "[ERROR CODE] 0x" << std::hex << std::uppercase << getErrorCode()
			<< std::dec << " (" << static_cast<unsigned long>(getErrorCode()) << ")" << std::endl
			<< "[ERROR STRING] " << getErrorString() << std::endl
			<< "[DESCRIPTION] " << getErrorDescription() << std::endl;

		if(!info.empty()){
			oss << "\n[ERRPR INFO]\n" << getErrorInfo() << std::endl << std::endl;
		}
		oss << getOriginString();

		whatBuffer = oss.str();
		return whatBuffer.c_str();
	}

	const char* DX11Exception::getType() const noexcept{
		return "Clove DX11 Exception";
	}

	HRESULT DX11Exception::getErrorCode() const noexcept{
		return hr;
	}

	std::string DX11Exception::getErrorString() const noexcept{
		return DXGetErrorStringA(hr);
	}

	std::string DX11Exception::getErrorDescription() const noexcept{
		char buff[512];
		DXGetErrorDescriptionA(hr, buff, sizeof(buff));
		return buff;
	}

	std::string DX11Exception::getErrorInfo() const noexcept{
		return info;
	}

	DeviceRemovedException::~DeviceRemovedException() = default;

	const char* DeviceRemovedException::getType() const noexcept{
		return "Clove DX11 Exception [DEVICE REMOVED] (GXGI_ERROR_DEVICE_REMOVED)";
	}

	InfoException::~InfoException() = default;

	InfoException::InfoException(int lineNum, const char* file, const std::vector<std::string>& messages)
		: CloveException(lineNum, file){

		for(const auto& message : messages){
			info += message;
			info.push_back('\n');
		}

		if(!info.empty()){
			info.pop_back();
		}
	}

	const char* InfoException::what() const noexcept{
		std::ostringstream oss;
		oss << getType() << std::endl
			<< "\n[ERROR INFO]\n" << getErrorInfo() << std::endl << std::endl;
		oss << getOriginString();
		whatBuffer = oss.str();
		return whatBuffer.c_str();
	}

	const char* InfoException::getType() const noexcept{
		return "Clove DX11 Info Exception";
	}

	std::string InfoException::getErrorInfo() const noexcept{
		return info;
	}
}