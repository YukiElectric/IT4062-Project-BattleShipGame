char* encrypt(const char *plaintext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;
    ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, (unsigned char *)SECRET_KEY, NULL);
    ciphertext_len = strlen(plaintext) + AES_BLOCK_SIZE;
    char *ciphertext = (char *)malloc(ciphertext_len);
    EVP_EncryptUpdate(ctx, (unsigned char *)ciphertext, &len, (const unsigned char *)plaintext, strlen(plaintext));
    ciphertext_len = len;
    EVP_EncryptFinal_ex(ctx, (unsigned char *)(ciphertext + len), &len);
    ciphertext_len += len;
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext;
}

char* decrypt(const char *ciphertext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;
    ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, (unsigned char *)SECRET_KEY, NULL);
    plaintext_len = strlen(ciphertext);
    char *plaintext = (char *)malloc(plaintext_len);
    EVP_DecryptUpdate(ctx, (unsigned char *)plaintext, &len, (const unsigned char *)ciphertext, strlen(ciphertext));
    plaintext_len = len;
    EVP_DecryptFinal_ex(ctx, (unsigned char *)(plaintext + len), &len);
    plaintext_len += len;
    EVP_CIPHER_CTX_free(ctx);
    return plaintext;
}