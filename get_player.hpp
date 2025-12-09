#ifndef GET_PLAYER_HPP
#define GET_PLAYER_HPP

#include "Player.hpp"
#include "World.hpp"
#include "ShaderProgram.hpp"
#include "glutils.hpp"

#include <memory>
#include <string>

#ifdef USE_CURL
#include <stdexcept>
#include <optional>
#include <cstdio>

#include "include/base64.hpp"
#include "include/json.hpp"

#include "NonCopyable.hpp"

#include <curl/curl.h>

/// A simple wrapper around CURL to download data
namespace webdl {
	namespace {
		/// curl callback to write data to a string
		size_t write_string(char* data, size_t size, size_t nmemb, void* ptr) {
			static_cast<std::string*>(ptr)->append(data, size*nmemb);
			return size*nmemb;
		}
	}

	class Connection final : NonCopyable {
		private:
			CURL* _handle;

		public:
			inline Connection() : _handle(curl_easy_init()) {
				if(!this->_handle) {
					throw std::runtime_error("Failed to initialize CURL");
				}
			}
			inline ~Connection() {
				curl_easy_cleanup(this->_handle);
			}

			inline std::string get(const std::string& url) const {
				std::string str;
				
				curl_easy_setopt(this->_handle, CURLOPT_URL, url.c_str());
				curl_easy_setopt(this->_handle, CURLOPT_WRITEFUNCTION, write_string);
				curl_easy_setopt(this->_handle, CURLOPT_WRITEDATA, &str);
				
				const CURLcode c = curl_easy_perform(this->_handle);

				if(c != CURLE_OK) {
					throw std::runtime_error(std::string("CURL ") + curl_easy_strerror(c) + " (" + std::to_string(c) + ")");
				}

				return str;
			}
	};
}

/// Loads a player's skin from online, uses fallback in event of error
std::shared_ptr<Player> get_player(World& world, const ShaderProgram& shader, glutils::TextureManager& tm, const std::string name) {
    if(name != "Idril" && !name.empty()) {
        try {
            const webdl::Connection conn;
            const std::string uuid = json::cast::string(json::cast::object(json::parse(conn.get(std::string("https://api.mojang.com/users/profiles/minecraft/") + name)))["id"]);
            const std::string payload = base64::decode(json::cast::string(json::cast::object(json::cast::list(json::cast::object(json::parse(conn.get(std::string("https://sessionserver.mojang.com/session/minecraft/profile/") + uuid)))["properties"])[0])["value"]));
            auto textures = json::cast::object(json::cast::object(json::parse(payload))["textures"]);
        
        
            const std::string skin = json::cast::string(json::cast::object(textures["SKIN"])["url"]);
            const bool slim = json::is::object(json::cast::object(textures["SKIN"])["metadata"]) && json::cast::string(json::cast::object(json::cast::object(textures["SKIN"])["metadata"])["model"]) == "slim";
            const std::string cape = json::is::object(textures["CAPE"]) ? json::cast::string(json::cast::object(textures["CAPE"])["url"]) : "";
        
            const std::string skin_data = conn.get(skin);
            const std::string cape_data = cape.empty() ? cape : conn.get(cape);
        
            return std::make_shared<Player>(world, shader, std::array<GLuint, 2> {tm.load("skin", std::vector<unsigned char>(skin_data.begin(), skin_data.end())), tm.load("assets/textures/dull.png")}, slim, cape_data.empty() ? std::nullopt : std::optional(std::array<GLuint, 2> {tm.load("cape", std::vector<unsigned char>(cape_data.begin(), cape_data.end())), tm.load("assets/textures/dull.png")}));
        } catch(const std::exception& exception) {
            fprintf(stderr, "[ERROR]: Unable to load player skin %s\n", exception.what());
        }
    }
    
    return std::make_shared<Player>(world, shader, std::array<GLuint, 2> {tm.load("assets/textures/idril.png"), tm.load("assets/textures/idril_specular.png")}, true, std::array<GLuint, 2> {tm.load("assets/textures/cape.png"), tm.load("assets/textures/cape_specular.png")});
}

#else
/// Fallback implementation to load internal Idril player
std::shared_ptr<Player> get_player(World& world, const ShaderProgram& shader, glutils::TextureManager& tm, const std::string _unused) {
    return std::make_shared<Player>(world, shader, std::array<GLuint, 2> {tm.load("assets/textures/idril.png"), tm.load("assets/textures/idril_specular.png")}, true, std::array<GLuint, 2> {tm.load("assets/textures/cape.png"), tm.load("assets/textures/cape_specular.png")});
}
#endif
#endif