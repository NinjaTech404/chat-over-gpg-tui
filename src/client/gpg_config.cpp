#ifndef GPG_CONFIG_HPP
#define GPG_CONFIG_HPP

#include <gpgme++/context.h>
#include <gpgme++/data.h>
#include <gpgme++/key.h>
#include <gpgme++/encryptionresult.h>
#include <gpgme++/decryptionresult.h>
#include <gpgme++/interfaces/passphraseprovider.h>

#include <stdexcept>
#include <string>
#include <cstring>
#include <vector>

namespace gpg {

class GetPassPhrase : public GpgME::PassphraseProvider {
  std::string passphrase;

  public: 
    GetPassPhrase(std::string passphrase_) : passphrase(std::move(passphrase_)) {}
    char* getPassphrase(const char*, const char*, bool, bool&) override;
};

char* GetPassPhrase::getPassphrase(const char* keyID, const char* userIdHint, bool prevWasBad, bool& cancel){

  if(prevWasBad){
    cancel = true;
    return nullptr;
  }

  cancel = false;

  char* buffer = static_cast<char*>(std::malloc(passphrase.size() + 1));

  if(buffer){
    std::strcpy(buffer, passphrase.c_str());
  }

  return buffer;

}

bool can_key_encrypt(const GpgME::Key& key) {
    // 1. Check null key
    if (key.isNull()) {
        throw std::runtime_error("[!] Key error\nKey object is null or unitialized.");
    }

    const char* fpr = key.primaryFingerprint();
    std::string primaryFingerprint = std::string("\nPrimary Key Fingerprint: ") + (fpr ? fpr : "N/A");

    if (key.isRevoked()) {
        throw std::runtime_error("[!] Key error\nPrimary key is revoked." + primaryFingerprint);
    }

    if (key.isDisabled()) {
        throw std::runtime_error("[!] Key error\nPrimary key is disabled." + primaryFingerprint);
    }

    if (key.isExpired()) {
        throw std::runtime_error("[!] Key error\nPrimary key has expired." + primaryFingerprint);
    }

    // 2. Fast-path check on top-level key capability
    if (key.canEncrypt()) {
      return true;
    }

    // 3. Iterate subkeys to check detailed subkey states
    if (key.numSubkeys() == 0) {
        throw std::runtime_error("[!] Key error\nKey contains no valid subkeys." + primaryFingerprint);
    }

    bool has_encrypt_capability = false;

    for (const GpgME::Subkey& subkey : key.subkeys()) {
        if (subkey.canEncrypt()) {
            has_encrypt_capability = true;
            
            if (subkey.isRevoked()) {
                throw std::runtime_error("[!] Key error\nEncryption subkey is revoked." + primaryFingerprint);
            }
            if (subkey.isExpired()) {
                throw std::runtime_error("[!] Key error\nEncryption subkey has expired." + primaryFingerprint);
            }
            if (subkey.isDisabled()) {
                throw std::runtime_error("[!] Key error\nEncryption subkey is disabled." + primaryFingerprint);
            }

            // Valid, active encryption subkey found
            return true; 
        }
    }

    if (!has_encrypt_capability) {
        throw std::runtime_error("[!] Key error\nKey lacks encryption capability." + primaryFingerprint);
    }

    throw std::runtime_error("[!] Key error\nKey cannot be used for encryption." + primaryFingerprint);
}

bool can_key_decrypt(const GpgME::Key& key) {
    // 1. Check null key
    if (key.isNull()) {
        throw std::runtime_error("[!] Key error\nKey object is null or unitialized.");
    }

    const char* fpr = key.primaryFingerprint();
    std::string primaryFingerprint = std::string("\nPrimary Key Fingerprint: ") + (fpr ? fpr : "N/A");
    
    if (key.isRevoked()) {
        throw std::runtime_error("[!] Key error\nPrimary key is revoked." + primaryFingerprint);
    }

    if (key.isDisabled()) {
        throw std::runtime_error("[!] Key error\nPrimary key is disabled." + primaryFingerprint);
    }

    if (key.isExpired()) {
        throw std::runtime_error("[!] Key error\nPrimary key has expired." + primaryFingerprint);
    }

    // 2. Check secret key status
    if (!key.hasSecret()) {
        throw std::runtime_error("[!] Key error\nProvided key is a public key, not a secret/private key." + primaryFingerprint);
    }

    // 3. Fast-path check on top-level key capability
    if (key.canEncrypt()) {
        return true;
    }

    // 4. Iterate subkeys to check detailed subkey states
    if (key.numSubkeys() == 0) {
        throw std::runtime_error("[!] Key error\nKey contains no valid subkeys." + primaryFingerprint);
    }

    bool has_decrypt_capability = false;

    for (const GpgME::Subkey& subkey : key.subkeys()) {
        if (subkey.canEncrypt()) {
            has_decrypt_capability = true;
            
            if (subkey.isRevoked()) {
                throw std::runtime_error("[!] Key error\nDecryption subkey is revoked." + primaryFingerprint);
            }
            if (subkey.isExpired()) {
                throw std::runtime_error("[!] Key error\nDecryption subkey has expired." + primaryFingerprint);
            }
            if (subkey.isDisabled()) {
                throw std::runtime_error("[!] Key error\nDecryption subkey is disabled." + primaryFingerprint);
            }

            // Valid, active decryption subkey found
            return true; 
        }
    }

    if (!has_decrypt_capability) {
        throw std::runtime_error("[!] Key error\nKey lacks decryption capability." + primaryFingerprint);
    }

    throw std::runtime_error("[!] Key error\nKey cannot be used for decryption." + primaryFingerprint);
}

std::string encrypt (const std::vector<GpgME::Key>& keys, const std::string& data){

  GpgME::initializeLibrary();

  std::unique_ptr<GpgME::Context> ctx = GpgME::Context::create(GpgME::OpenPGP);

  if(!ctx){
    throw std::runtime_error("[!] Runtime Error\nFailed to start the GpgME OpenPGP context.");
  }

  if(keys.size() > 0){
    for(GpgME::Key key : keys){
      if(!can_key_encrypt(key)) throw std::runtime_error(std::string("[!] Invalid Public Key\nThe GnuGP public key lacks encryption capability\nKey fingerprint: ") + key.primaryFingerprint());
    }
  }
  else throw std::runtime_error("[!] Invalid Public Key\nThere are no public key to encrypt the data.");
  
  if(data.empty()){
    throw std::runtime_error("[!] Invalid Data\nEmpty Message Data");
  }

  GpgME::Data in(data.data(), data.size(), false);
  GpgME::Data out;

  GpgME::EncryptionResult result = ctx->encrypt(keys, in, out, GpgME::Context::AlwaysTrust);
  if(result.error()){
    throw std::runtime_error(std::string("[!] Encryption Failed\n") + gpgme_strerror(result.error().encodedError()));
  }

  out.seek(0, SEEK_SET);
  std::string plainText = out.toString();

  if(plainText.empty()) throw std::runtime_error("[!] Invalid Encryption\nDecrypted output is empty or corrupted payload.");

  return out.toString();

}

std::string decrypt (const std::string& data, const std::string& fingerprint, const std::string& passphrase){

  GpgME::initializeLibrary();

  std::unique_ptr<GpgME::Context> ctx = GpgME::Context::create(GpgME::OpenPGP);
  std::unique_ptr<GetPassPhrase> passphrase_provider = std::make_unique<GetPassPhrase>(passphrase);

  if(!ctx){
    throw std::runtime_error("[!] Runtime Error\nFaild to start the GpgME OpenPGP context.");
  }

  GpgME::Error err;
  GpgME::Key key = ctx->key(fingerprint.data(), err, true);
  
  if(err || key.isNull()) throw std::runtime_error(std::string("[!] Invalid Secret Key\nNo secret key found associated with fingerprint: ") + fingerprint);

  if(data.empty()) throw std::runtime_error("[!] Invalid Data\nEmpty Message Data");

  bool isValidBinary = (static_cast<unsigned char>(data[0]) & 0x80) != 0;
  if(!isValidBinary)  throw std::runtime_error("[!] Invalid Data\nInvalid PGP binary data.");
  
  if (!passphrase.empty()) {
    ctx->setPinentryMode(GpgME::Context::PinentryLoopback);
    ctx->setPassphraseProvider(passphrase_provider.get());
  }
  else throw std::runtime_error(std::string("[!] Invalid Passphrase\nPassphrase of the secret key is required for decryption\nkey fingerprint: ") + fingerprint);

  GpgME::Data in(data.data(), data.size(), false);
  GpgME::Data out;
  
  if(can_key_decrypt(key)){
    GpgME::DecryptionResult result = ctx->decrypt(in, out);
    if(result.error()){
      throw std::runtime_error(std::string("Decryption Failed: ") + gpgme_strerror(result.error().encodedError()));
    }
  }

  out.seek(0, SEEK_SET);
  std::string plainText = out.toString();
  
  if(plainText.empty()) throw std::runtime_error("[!] Invalid Decryption\nDecrypted output is empty or corrupted payload.");

  return plainText;
}

}

#endif // !GPG_CONFIG_HPP

