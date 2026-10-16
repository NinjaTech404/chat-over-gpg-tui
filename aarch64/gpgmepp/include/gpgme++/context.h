/*
  context.h - wraps a gpgme key context
  Copyright (C) 2003, 2007 Klarälvdalens Datakonsult AB

  This file is part of GPGME++.

  GPGME++ is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  GPGME++ is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with GPGME++; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

// -*- c++ -*-
#ifndef __GPGMEPP_CONTEXT_H__
#define __GPGMEPP_CONTEXT_H__

#include "global.h"

#include "error.h"
#include "key.h"
#include "verificationresult.h" // for Signature::Notation

#include <memory>
#include <string>
#include <type_traits>
#include <vector>
#include <utility>
#include <iosfwd>

namespace GpgME
{

class Data;
class ProgressProvider;
class PassphraseProvider;
class EventLoopInteractor;
class EditInteractor;
class AssuanTransaction;

class KeyListResult;
class KeyGenerationResult;
class ImportResult;
class DecryptionResult;
class VerificationResult;
class SigningResult;
class EncryptionResult;
class VfsMountResult;
class RandomBytesResult;
class RandomValueResult;
class RandomZBase32StringResult;

class EngineInfo;

class GPGMEPP_EXPORT Context
{
    explicit Context(gpgme_ctx_t);
public:
    //using GpgME::Protocol;

    /// RAII-style class for saving/restoring the key list mode.
    class GPGMEPP_EXPORT KeyListModeSaver
    {
    public:
        explicit KeyListModeSaver(Context *ctx);
        ~KeyListModeSaver();
    private:
        Context *mCtx;
        unsigned int mKeyListMode;
    };

    //
    // Creation and destruction:
    //

    static Context *createForProtocol(Protocol proto);
    /** Same as above but returning a unique ptr. */
    static std::unique_ptr<Context> create(Protocol proto);
    static std::unique_ptr<Context> createForEngine(Engine engine, Error *err = nullptr);
    virtual ~Context();

    //
    // Context Attributes
    //

    Protocol protocol() const;

    void setArmor(bool useArmor);
    bool armor() const;

    void setTextMode(bool useTextMode);
    bool textMode() const;

    void setOffline(bool useOfflineMode);
    bool offline() const;

    const char *getFlag(const char *name) const;
    Error setFlag(const char *name, const char *value);

    enum CertificateInclusion {
        DefaultCertificates = -256,
        AllCertificatesExceptRoot = -2,
        AllCertificates = -1,
        NoCertificates = 0,
        OnlySenderCertificate = 1
    };
    void setIncludeCertificates(int which);
    int includeCertificates() const;

    //using GpgME::KeyListMode;
    void setKeyListMode(unsigned int keyListMode);
    void addKeyListMode(unsigned int keyListMode);
    unsigned int keyListMode() const;

    /** Set the passphrase provider
     *
     * To avoid problems where a class using a context registers
     * itself as the provider the Context does not take ownership
     * of the provider and the caller must ensure that the provider
     * is deleted if it is no longer needed.
     */
    void setPassphraseProvider(PassphraseProvider *provider);
    PassphraseProvider *passphraseProvider() const;

    /** Set the progress provider
     *
     * To avoid problems where a class using a context registers
     * itself as the provider the Context does not take ownership
     * of the provider and the caller must ensure that the provider
     * is deleted if it is no longer needed.
     */
    void setProgressProvider(ProgressProvider *provider);
    ProgressProvider *progressProvider() const;

    void setManagedByEventLoopInteractor(bool managed);
    bool managedByEventLoopInteractor() const;

    GpgME::Error setLocale(int category, const char *value);

    EngineInfo engineInfo() const;
    GpgME::Error setEngineFileName(const char *filename);
    GpgME::Error setEngineHomeDirectory(const char *filename);

    enum PinentryMode{
        PinentryDefault = 0,
        PinentryAsk = 1,
        PinentryCancel = 2,
        PinentryError = 3,
        PinentryLoopback = 4
    };
    GpgME::Error setPinentryMode(PinentryMode which);
    PinentryMode pinentryMode() const;

private:
    friend class ::GpgME::EventLoopInteractor;
    void installIOCallbacks(gpgme_io_cbs *iocbs);
    void uninstallIOCallbacks();

public:
    //
    //
    // Key Management
    //
    //

    //
    // Key Listing
    //

    GpgME::Error startKeyListing(const char *pattern = nullptr, bool secretOnly = false);
    GpgME::Error startKeyListing(const char *patterns[], bool secretOnly = false);

    Key nextKey(GpgME::Error &e);

    KeyListResult endKeyListing();
    KeyListResult keyListResult() const;

    Key key(const char *fingerprint, GpgME::Error &e, bool secret = false);

    //
    // Key Generation
    //

    KeyGenerationResult generateKey(const char *parameters, Data &pubKey);
    GpgME::Error startKeyGeneration(const char *parameters, Data &pubkey);
    KeyGenerationResult keyGenerationResult() const;

    //
    // Key Export
    //
    enum ExportMode {
        ExportDefault = 0,
        ExportExtern = 2,
        ExportMinimal = 4,
        ExportSecret = 16,
        ExportRaw = 32,
        ExportPKCS12 = 64,
        ExportNoUID = 128, // obsolete; has no effect
        ExportSSH = 256,
        ExportSecretSubkey = 512,
    };

    GpgME::Error exportPublicKeys(const char *pattern, Data &keyData);
    GpgME::Error exportPublicKeys(const char *pattern, Data &keyData, unsigned int mode);
    GpgME::Error exportPublicKeys(const char *pattern[], Data &keyData);
    GpgME::Error exportPublicKeys(const char *pattern[], Data &keyData, unsigned int mode);
    GpgME::Error startPublicKeyExport(const char *pattern, Data &keyData);
    GpgME::Error startPublicKeyExport(const char *pattern, Data &keyData, unsigned int mode);
    GpgME::Error startPublicKeyExport(const char *pattern[], Data &keyData);
    GpgME::Error startPublicKeyExport(const char *pattern[], Data &keyData, unsigned int mode);

    GpgME::Error exportSecretKeys(const char *pattern, Data &keyData, unsigned int mode = ExportSecret);
    GpgME::Error exportSecretKeys(const char *pattern[], Data &keyData, unsigned int mode = ExportSecret);
    GpgME::Error startSecretKeyExport(const char *pattern, Data &keyData, unsigned int mode = ExportSecret);
    GpgME::Error startSecretKeyExport(const char *pattern[], Data &keyData, unsigned int mode = ExportSecret);

    GpgME::Error exportSecretSubkeys(const char *pattern, Data &keyData, unsigned int mode = ExportSecretSubkey);
    GpgME::Error exportSecretSubkeys(const char *pattern[], Data &keyData, unsigned int mode = ExportSecretSubkey);
    GpgME::Error startSecretSubkeyExport(const char *pattern, Data &keyData, unsigned int mode = ExportSecretSubkey);
    GpgME::Error startSecretSubkeyExport(const char *pattern[], Data &keyData, unsigned int mode = ExportSecretSubkey);

    // generic export functions; prefer using the specific public/secret key export functions
    GpgME::Error exportKeys(const char *pattern, Data &keyData, unsigned int mode = ExportDefault);
    GpgME::Error exportKeys(const char *pattern[], Data &keyData, unsigned int mode = ExportDefault);
    GpgME::Error startKeyExport(const char *pattern, Data &keyData, unsigned int mode = ExportDefault);
    GpgME::Error startKeyExport(const char *pattern[], Data &keyData, unsigned int mode = ExportDefault);

    //
    // Key Import
    //

    ImportResult importKeys(const Data &data);
    ImportResult importKeys(const std::vector<Key> &keys);
    ImportResult importKeys(const std::vector<std::string> &keyIds);
    GpgME::Error startKeyImport(const Data &data);
    GpgME::Error startKeyImport(const std::vector<Key> &keys);
    GpgME::Error startKeyImport(const std::vector<std::string> &keyIds);
    ImportResult importResult() const;

    //
    // Key Deletion
    //

    GpgME::Error deleteKey(const Key &key, DeletionFlags flags);
    GpgME::Error startKeyDeletion(const Key &key, DeletionFlags flags);

    //
    // Passphrase changing
    //

    GpgME::Error passwd(const Key &key);
    GpgME::Error startPasswd(const Key &key);

    //
    // Key Editing
    //

    GpgME::Error edit(const Key &key, std::unique_ptr<EditInteractor> function, Data &out);
    GpgME::Error startEditing(const Key &key, std::unique_ptr<EditInteractor> function, Data &out);


    //
    // Modern Interface actions. Require 2.1.x
    //

    enum CreationFlags : unsigned int {
        // Keep in line with core's GPGME_CREATE_* flags
        CreateUseDefaults  = 0,
        CreateSign         = (1 << 0),  /* Allow usage: signing. */
        CreateEncrypt      = (1 << 1),  /* Allow usage: encryption. */
        CreateCertify      = (1 << 2),  /* Allow usage: certification. */
        CreateAuthenticate = (1 << 3),  /* Allow usage: authentication. */
        CreateNoPassword   = (1 << 7),  /* Create w/o passphrase. */
        CreateForce        = (1 << 12), /* Force creation if key with same user ID already exists. */
        CreateNoExpire     = (1 << 13), /* Create w/o expiration. */
        CreateADSK         = (1 << 14), /* Add an ADSK. */
        CreateGroupKey     = (1 << 15), /* Flag as group key. Needs gpg 2.2.48, 2.4.8, or 2.5.7+. */
    };

    /*!
     * Create a new OpenPGP key.
     *
     * Creates a new OpenPGP key with user ID \a userid. \a algo needs to be in a format
     * understood by `gpg --quick-gen-key`. If \a expires is 0 then the new key will have
     * the default expiration unless the CreateNoExpire flag is set. If \a flags contains
     * neither one of the usage flags nor the CreateGroupKey flag then an OpenPGP key with
     * standard layout (primary signing and certification key plus encryption subkey) is
     * created. Otherwise, a primary key with the specified usage is created.
     */
    GpgME::KeyGenerationResult createKey(const std::string &userid,
                                         const std::string &algo = {},
                                         unsigned long expires = 0,
                                         CreationFlags flags = CreateUseDefaults);
    /*!
     * Starts the creation of a new OpenPGP key.
     *
     * \sa createKey
     */
    Error startCreateKey(const std::string &userid,
                         const std::string &algo = {},
                         unsigned long expires = 0,
                         CreationFlags flags = CreateUseDefaults);

    Error addUid(const Key &key, const char *userid);
    Error startAddUid(const Key &key, const char *userid);

    Error revUid(const Key &key, const char *userid);
    Error startRevUid(const Key &key, const char *userid);

    Error setPrimaryUid(const Key &key, const char *userid);
    Error startSetPrimaryUid(const Key &key, const char *userid);

    /*!
     * Create a new subkey for a given OpenPGP key.
     *
     * Creates a new subkey for the key \a key. \a algo needs to be in a format
     * understood by `gpg --quick-gen-key`. If \a expires is 0 then the new key will have
     * the default expiration unless the CreateNoExpire flag is set.
     */
    KeyGenerationResult createSubkey(const Key &key,
                                     const std::string &algo = {},
                                     unsigned long expires = 0,
                                     CreationFlags flags = CreateUseDefaults);
    /*!
     * Start the creation a new subkey for a given OpenPGP key.
     *
     * \sa createSubkey
     */
    Error startCreateSubkey(const Key &key,
                            const std::string &algo = {},
                            unsigned long expires = 0,
                            CreationFlags flags = CreateUseDefaults);

    enum SetExpireFlags {
        SetExpireDefault = 0,
        SetExpireAllSubkeys = 1
    };

    Error setExpire(const Key &k, unsigned long expires,
                    const std::vector<Subkey> &subkeys = std::vector<Subkey>(),
                    const SetExpireFlags flags = SetExpireDefault);
    Error startSetExpire(const Key &k, unsigned long expires,
                         const std::vector<Subkey> &subkeys = std::vector<Subkey>(),
                         const SetExpireFlags flags = SetExpireDefault);

    /**
     * Sets the owner trust of the key \a key to the value \a trust.
     * Requires gpg 2.4.6.
     */
    Error setOwnerTrust(const Key &key, Key::OwnerTrust trust);
    /**
     * Starts the operation to set the owner trust of the key \a key to the value \a trust.
     * Requires gpg 2.4.6.
     */
    Error startSetOwnerTrust(const Key &key, Key::OwnerTrust trust);

    /**
     * Enables or disables the key \a key.
     * Requires gpg 2.4.6.
     */
    Error setKeyEnabled(const Key &key, bool enabled);
    /**
     * Starts the operation to enable or disable the key \a key.
     * Requires gpg 2.4.6.
     */
    Error startSetKeyEnabled(const Key &key, bool enabled);

    Error revokeSignature(const Key &key, const Key &signingKey,
                          const std::vector<UserID> &userIds = std::vector<UserID>());
    Error startRevokeSignature(const Key &key, const Key &signingKey,
                               const std::vector<UserID> &userIds = std::vector<UserID>());

    Error addAdsk(const Key &k, const char *adsk);
    Error startAddAdsk(const Key &k, const char *adsk);

    // using TofuInfo::Policy
    Error setTofuPolicy(const Key &k, unsigned int policy);
    Error setTofuPolicyStart(const Key &k, unsigned int policy);

    EditInteractor *lastEditInteractor() const;
    std::unique_ptr<EditInteractor> takeLastEditInteractor();

    //
    // SmartCard Editing
    //

    GpgME::Error cardEdit(const Key &key, std::unique_ptr<EditInteractor> function, Data &out);
    GpgME::Error startCardEditing(const Key &key, std::unique_ptr<EditInteractor> function, Data &out);

    EditInteractor *lastCardEditInteractor() const;
    std::unique_ptr<EditInteractor> takeLastCardEditInteractor();

    //
    // Assuan Transactions
    //

    GpgME::Error assuanTransact(const char *command, std::unique_ptr<AssuanTransaction> transaction);
    GpgME::Error assuanTransact(const char *command);
    GpgME::Error startAssuanTransaction(const char *command, std::unique_ptr<AssuanTransaction> transaction);
    GpgME::Error startAssuanTransaction(const char *command);

    AssuanTransaction *lastAssuanTransaction() const;
    std::unique_ptr<AssuanTransaction> takeLastAssuanTransaction();

    //
    //
    // Crypto Operations
    //

    enum DecryptionFlags {
        // Keep in line with core's flags
        DecryptNone = 0,
        DecryptVerify = 1,
        DecryptArchive = 2,
        DecryptListOnly = 16,
        DecryptUnwrap = 128,
        DecryptMaxValue = 0x80000000
    };

    //
    // Decryption
    //

    // Alternative way to set decryption flags as they were added only in
    // 1.9.0 and so other API can still be used but with 1.9.0 additionally
    // flags can be set.
    void setDecryptionFlags (const DecryptionFlags flags);

    DecryptionResult decrypt(const Data &cipherText, Data &plainText);
    GpgME::Error startDecryption(const Data &cipherText, Data &plainText);
    DecryptionResult decrypt(const Data &cipherText, Data &plainText, const DecryptionFlags flags);
    GpgME::Error startDecryption(const Data &cipherText, Data &plainText, const DecryptionFlags flags);
    DecryptionResult decryptionResult() const;

    //
    // Signature Verification
    //

    VerificationResult verifyDetachedSignature(const Data &signature, const Data &signedText);
    VerificationResult verifyOpaqueSignature(const Data &signedData, Data &plainText);
    GpgME::Error startDetachedSignatureVerification(const Data &signature, const Data &signedText);
    GpgME::Error startOpaqueSignatureVerification(const Data &signedData, Data &plainText);
    VerificationResult verificationResult() const;

    //
    // Combined Decryption and Signature Verification
    //

    std::pair<DecryptionResult, VerificationResult> decryptAndVerify(const Data &cipherText, Data &plainText);
    std::pair<DecryptionResult, VerificationResult> decryptAndVerify(const Data &cipherText, Data &plainText, const DecryptionFlags flags);
    GpgME::Error startCombinedDecryptionAndVerification(const Data &cipherText, Data &plainText);
    GpgME::Error startCombinedDecryptionAndVerification(const Data &cipherText, Data &plainText, const DecryptionFlags flags);
    // use verificationResult() and decryptionResult() to retrieve the result objects...

    //
    // Signing
    //

    void clearSigningKeys();
    GpgME::Error addSigningKey(const Key &signer);
    Key signingKey(unsigned int index) const;
    std::vector<Key> signingKeys() const;

    void clearSignatureNotations();
    GpgME::Error addSignatureNotation(const char *name, const char *value, unsigned int flags = 0);
    GpgME::Error addSignaturePolicyURL(const char *url, bool critical = false);
    const char *signaturePolicyURL() const;
    Notation signatureNotation(unsigned int index) const;
    std::vector<Notation> signatureNotations() const;

    //using GpgME::SignatureMode;
    SigningResult sign(const Data &plainText, Data &signature, SignatureMode mode);
    GpgME::Error startSigning(const Data &plainText, Data &signature, SignatureMode mode);
    SigningResult signingResult() const;

    // wrapper for gpgme_set_sender
    const char *getSender();
    GpgME::Error setSender(const char *sender);

    //
    // Encryption
    //

    enum EncryptionFlags {
        None = 0,
        AlwaysTrust = 1,
        NoEncryptTo = 2,
        Prepare = 4,
        ExpectSign = 8,
        NoCompress = 16,
        Symmetric = 32,
        ThrowKeyIds = 64,
        EncryptWrap = 128,
        WantAddress = 256,
        EncryptArchive = 512,
        EncryptFile = 1024,
        AddRecipient = 2048,
        ChangeRecipient = 4096,
    };
    EncryptionResult encrypt(const std::vector<Key> &recipients, const Data &plainText, Data &cipherText, EncryptionFlags flags);
    GpgME::Error encryptSymmetrically(const Data &plainText, Data &cipherText);
    GpgME::Error startEncryption(const std::vector<Key> &recipients, const Data &plainText, Data &cipherText, EncryptionFlags flags);
    EncryptionResult encryptionResult() const;

    //
    // Combined Signing and Encryption
    //

    std::pair<SigningResult, EncryptionResult> signAndEncrypt(const std::vector<Key> &recipients, const Data &plainText, Data &cipherText, EncryptionFlags flags);
    GpgME::Error startCombinedSigningAndEncryption(const std::vector<Key> &recipients, const Data &plainText, Data &cipherText, EncryptionFlags flags);
    // use encryptionResult() and signingResult() to retrieve the result objects...

    //
    //
    // Audit Log
    //
    //
    enum AuditLogFlags {
        DefaultAuditLog = 0,
        HtmlAuditLog = 1,
        DiagnosticAuditLog = 2,
        AuditLogWithHelp = 128
    };
    GpgME::Error startGetAuditLog(Data &output, unsigned int flags = 0);
    GpgME::Error getAuditLog(Data &output, unsigned int flags = 0);

    //
    //
    // Random values
    //
    //
    enum class GPGMEPP_DEPRECATED RandomMode {
        Normal = 0,
        ZBase32 = 1
    };
    /*!
     * Generate random bytes.
     *
     * This function creates a buffer of \a count bytes and fills this buffer by calling
     * gpgme_op_random_bytes. \a count must be at most \c 1024. If \a mode is \c ZBase32
     * then \a count must be at least \c 31. In this case the buffer is filled with 30
     * ASCII characters followed by a null byte; the remainder of the buffer is uninitialized.
     *
     * This function is deprecated. For \c Normal mode use the other overload of
     * generateRandomBytes. For \c ZBase32 mode use generateRandomZBase32String.
     */
    GPGMEPP_DEPRECATED RandomBytesResult generateRandomBytes(size_t count, RandomMode mode);

    /*!
     * Generate random bytes.
     *
     * This function creates a buffer of \a count bytes and fills this buffer with random
     * bytes retrieved from gpg. \a count must be at most \c 1024.
     */
    RandomBytesResult generateRandomBytes(size_t count);

    /*!
     * Generate an unbiased random value in the range [0, \a limit).
     */
    RandomValueResult generateRandomValue(unsigned int limit);

    /*!
     * Generate a string with 30 random z-base-32 characters.
     */
    RandomZBase32StringResult generateRandomZBase32String();

    //
    //
    // G13 crypto container operations
    //
    //
    GpgME::Error createVFS(const char *containerFile, const std::vector<Key> &recipients);
    VfsMountResult mountVFS(const char *containerFile, const char *mountDir);

    // Spawn Engine
    enum SpawnFlags {
        SpawnNone = 0,
        SpawnDetached = 1,
        SpawnAllowSetFg = 2,
        SpawnShowWindow = 4
    };
    /** Spwan the process \a file with arguments \a argv.
     *
     *  If a data parameter is null the /dev/null will be
     *  used. (Or other platform stuff).
     *
     * @param file The executable to start.
     * @param argv list of arguments file should be argv[0].
     * @param input The data to be sent through stdin.
     * @param output The data to be receive the stdout.
     * @param err The data to receive stderr.
     * @param flags Additional flags.
     *
     * @returns An error or empty error.
     */
    GpgME::Error spawn(const char *file, const char *argv[],
                       Data &input, Data &output, Data &err,
                       SpawnFlags flags);
    /** Async variant of spawn. Immediately returns after starting the
     * process. */
    GpgME::Error spawnAsync(const char *file, const char *argv[],
                            Data &input, Data &output,
                            Data &err, SpawnFlags flags);
    //
    //
    // Run Control
    //
    //

    bool poll();
    GpgME::Error wait();
    GpgME::Error lastError() const;
    GpgME::Error cancelPendingOperation();
    GpgME::Error cancelPendingOperationImmediately();

    class Private;
    const Private *impl() const
    {
        return d;
    }
    Private *impl()
    {
        return d;
    }

    GPGMEPP_DEPRECATED Error startCreateKey(const char *userid,
                                            const char *algo,
                                            unsigned long reserved,
                                            unsigned long expires,
                                            const Key &certkey,
                                            unsigned int flags);
    GPGMEPP_DEPRECATED Error createKey(const char *userid,
                                       const char *algo,
                                       unsigned long reserved,
                                       unsigned long expires,
                                       const Key &certkey,
                                       unsigned int flags);
    GPGMEPP_DEPRECATED GpgME::KeyGenerationResult createKeyEx(const char *userid,
                                                              const char *algo,
                                                              unsigned long reserved,
                                                              unsigned long expires,
                                                              const Key &certkey,
                                                              unsigned int flags);
    GPGMEPP_DEPRECATED Error createSubkey(const Key &key,
                                          const char *algo,
                                          unsigned long reserved,
                                          unsigned long expires,
                                          unsigned int flags);
    GPGMEPP_DEPRECATED Error startCreateSubkey(const Key &key,
                                               const char *algo,
                                               unsigned long reserved,
                                               unsigned long expires,
                                               unsigned int flags);
    GPGMEPP_DEPRECATED GpgME::Error deleteKey(const Key &key, bool allowSecretKeyDeletion = false);
    GPGMEPP_DEPRECATED GpgME::Error startKeyDeletion(const Key &key, bool allowSecretKeyDeletion = false);

private:
    // Helper functions that need to be context because they rely
    // on the "Friendlyness" of context to access the gpgme types.
    gpgme_key_t *getKeysFromRecipients(const std::vector<Key> &recipients);

private:
    Private *const d;

private: // disable...
    Context(const Context &);
    const Context &operator=(const Context &);
};

GPGMEPP_EXPORT std::ostream &operator<<(std::ostream &os, Context::CertificateInclusion incl);
GPGMEPP_EXPORT std::ostream &operator<<(std::ostream &os, Context::EncryptionFlags flags);
GPGMEPP_EXPORT std::ostream &operator<<(std::ostream &os, Context::AuditLogFlags flags);

inline Context::CreationFlags operator|(Context::CreationFlags lh, Context::CreationFlags rh)
{
    return static_cast<Context::CreationFlags>(
        std::underlying_type_t<Context::CreationFlags>(lh) |
        std::underlying_type_t<Context::CreationFlags>(rh));
}
inline Context::CreationFlags &operator|=(Context::CreationFlags &lh, Context::CreationFlags rh)
{
    return (Context::CreationFlags &)(
        (std::underlying_type_t<Context::CreationFlags> &)lh |=
        std::underlying_type_t<Context::CreationFlags>(rh));
}

} // namespace GpgME

#endif // __GPGMEPP_CONTEXT_H__
