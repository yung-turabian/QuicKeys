#include <locale.h>
#include <gpgme.h>

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define fail_if_err(err)					\
  do								\
    {								\
      if (err)							\
        {							\
          fprintf (stderr, "%s:%d: %s: %s\n",			\
                   __FILE__, __LINE__, gpgme_strsource (err),	\
		   gpgme_strerror (err));			\
          exit (1);						\
        }							\
    }								\
  while (0)


void init_gpgme(gpgme_protocol_t proto)
{
		gpgme_error_t err;

		setlocale(LC_ALL, "");
		gpgme_check_version(NULL);
		gpgme_set_locale(NULL, LC_CTYPE, setlocale(LC_CTYPE, NULL));
#ifndef HAVE_W32_SYSTEM
		gpgme_set_locale(NULL, LC_MESSAGES, setlocale(LC_MESSAGES, NULL));
#endif

		err = gpgme_engine_check_version(proto);
		fail_if_err(err);
}

gpgme_error_t generate_keys (gpgme_ctx_t ctx,const char **parmss,int sz,char **fprs,
  void (*progress_meter) (void *hook, const char *what, int type, int current,
                          int total))
{
  gpgme_set_progress_cb (ctx, progress_meter, NULL);
  gpgme_error_t err;
  gpgme_genkey_result_t result;

  int i,len;
  for (i = 0; i < sz;i++)
    {
      err = gpgme_op_genkey (ctx, parmss[i], NULL, NULL);
      fail_if_err(err);

      if (!fprs)
        continue;

      result = gpgme_op_genkey_result(ctx);
      /* Finger print must be stored for later use. Otherwise,
         it will be invalid since gpgme_op_genkey() might be called
         multiple times. */
      if (!result->fpr)
        fprs[i] = NULL;
      else {
        len = strlen(result->fpr);
        fprs[i] = (char *)malloc(sizeof(char) * (len + 1));
        memcpy(fprs[i],result->fpr,len);
        fprs[i][len] = '\0';
      }
    }
  return err;
}

gpgme_error_t generate_test_keys (gpgme_ctx_t ctx,int n,char **fprs,
  void (*progress_meter) (void *hook, const char *what, int type, int current,
                          int total))
{
  const char *parms1 = "<GnupgKeyParms format=\"internal\">\n"
    "Key-Type: RSA\n"
    "Key-Length: 2048\n"
    "Subkey-Type: RSA\n"
    "Subkey-Length: 2048\n"
    "Name-Real: Joe Tester\n"
    "Name-Comment: with stupid passphrase\n"
    "Name-Email: joe@foo.bar\n"
    "Expire-Date: 0\n"
    "Passphrase: abc\n"
    "</GnupgKeyParms>\n";

  const char *parms2 = "<GnupgKeyParms format=\"internal\">\n"
    "Key-Type: RSA\n"
    "Key-Length: 2048\n"
    "Subkey-Type: RSA\n"
    "Subkey-Length: 2048\n"
    "Name-Real: Joe Tester 2\n"
    "Name-Comment: with stupid passphrase 2\n"
    "Name-Email: joe2@foo.bar\n"
    "Expire-Date: 0\n"
    "Passphrase: abc\n"
    "</GnupgKeyParms>\n";

  const char *parms3 = "<GnupgKeyParms format=\"internal\">\n"
    "Key-Type: RSA\n"
    "Key-Length: 2048\n"
    "Subkey-Type: RSA\n"
    "Subkey-Length: 2048\n"
    "Name-Real: Joe Tester 3\n"
    "Name-Comment: with stupid passphrase 3\n"
    "Name-Email: joe2@foo.bar\n"
    "Expire-Date: 0\n"
    "Passphrase: abc\n"
    "</GnupgKeyParms>\n";

  switch (n){
    case 1:
    case 2:
    case 3:
      break;
    default:
      n = 3;
      break;
  }

  const char *parmss[n];
  switch (n){
    case 1:
      parmss[0] = parms1;
      break;
    case 2:
      parmss[0] = parms1;
      parmss[1] = parms2;
      break;
    case 3:
      parmss[0] = parms1;
      parmss[1] = parms2;
      parmss[2] = parms3;
      break;
    default:
      break;
  }
  return generate_keys (ctx,parmss,n,fprs,progress_meter);
}

gpgme_error_t generate_test_key (gpgme_ctx_t ctx,
  void (*progress_meter) (void *hook, const char *what, int type, int current,
                          int total))
{
  return generate_test_keys (ctx,1,NULL,progress_meter);
}

gpgme_error_t delete_test_key (gpgme_ctx_t ctx,gpgme_key_t key)
{
  gpgme_error_t err = gpgme_op_delete (ctx,key,1);
  gpgme_key_unref (key);
  return err;
}


void print_data (gpgme_data_t dh)
{
#define BUF_SIZE 512
  char buf[BUF_SIZE + 1];
  int ret;
  
  ret = gpgme_data_seek (dh, 0, SEEK_SET);
  if (ret)
    fail_if_err (gpgme_err_code_from_errno (errno));
  while ((ret = gpgme_data_read (dh, buf, BUF_SIZE)) > 0)
    fwrite (buf, ret, 1, stdout);
  if (ret < 0)
    fail_if_err (gpgme_err_code_from_errno (errno));

  /* Reset read position to the beginning so that dh can be used as input
     for another operation after this method call. For example, dh is an
     output from encryption and also is used as an input for decryption.
     Otherwise GPG_ERR_NO_DATA is returned since this method moves the
     read position. */
  ret = gpgme_data_seek (dh, 0, SEEK_SET);
}


int main()
{
		gpgme_ctx_t ctx;
		gpgme_error_t err;
		gpgme_data_t in, out;
		gpgme_key_t key[3] = {NULL, NULL, NULL};
		gpgme_encrypt_result_t result;


		init_gpgme(GPGME_PROTOCOL_OpenPGP);
		err = gpgme_new(&ctx);
		fail_if_err(err);
		gpgme_set_armor(ctx, 1);

		char *fprs[2];
		err = generate_test_keys(ctx, 2, fprs, NULL);
		fail_if_err(err);

		err = gpgme_data_new_from_mem(&in, "Shalom Olam\n", 12, 0);
		fail_if_err(err);

		err = gpgme_data_new(&out);
		fail_if_err(err);

		err = gpgme_get_key(ctx, fprs[0], &key[0], 0);
		fail_if_err(err);
		err = gpgme_get_key(ctx, fprs[1], &key[1], 0);
		fail_if_err(err);

		err = gpgme_op_encrypt(ctx, key, GPGME_ENCRYPT_ALWAYS_TRUST, in, out);
		fail_if_err(err);
		result = gpgme_op_encrypt_result(ctx);
		if(result->invalid_recipients) {
				fprintf(stderr, 
								"Invalid recipient encountered: %s\n", result->invalid_recipients->fpr);
				exit(1);
		}

		print_data(out);

		free(fprs[0]);
		free(fprs[1]);
		delete_test_key(ctx, key[0]);
		delete_test_key(ctx, key[1]);
		gpgme_data_release(in);
		gpgme_data_release(out);
		gpgme_release(ctx);

		return 0;
}
