/**************************************************
* File name: test_sm2_sign_and_verify.c
* Author: HAN Wei
* Author's blog: https://blog.csdn.net/henter/
* Date: Nov 20th, 2018
* Description: implement SM2 sign data and verify
    signature test functions
**************************************************/

#include <stdio.h>
#include <string.h>
#include "sm2_create_key_pair.h"
#include "sm2_sign_and_verify.h"
#include "test_sm2_sign_and_verify.h"

/*********************************************************/
int test_with_input_defined_in_standard(void)
{
	int error_code;
	unsigned char msg[] = {"message digest"};
	unsigned int msg_len = (unsigned int)(strlen((char *)msg));
	unsigned char user_id[] = {"1234567812345678"};
	unsigned int user_id_len = (unsigned int)(strlen((char *)user_id));
	unsigned char pub_key[65] = {0x04, 0x09, 0xf9, 0xdf, 0x31, 0x1e, 0x54, 0x21, 0xa1,
	                             0x50, 0xdd, 0x7d, 0x16, 0x1e, 0x4b, 0xc5, 0xc6,
				     0x72, 0x17, 0x9f, 0xad, 0x18, 0x33, 0xfc, 0x07,
				     0x6b, 0xb0, 0x8f, 0xf3, 0x56, 0xf3, 0x50, 0x20,
				     0xcc, 0xea, 0x49, 0x0c, 0xe2, 0x67, 0x75, 0xa5,
				     0x2d, 0xc6, 0xea, 0x71, 0x8c, 0xc1, 0xaa, 0x60,
				     0x0a, 0xed, 0x05, 0xfb, 0xf3, 0x5e, 0x08, 0x4a,
				     0x66, 0x32, 0xf6, 0x07, 0x2d, 0xa9, 0xad, 0x13};
	unsigned char pri_key[32] = {0x39, 0x45, 0x20, 0x8f, 0x7b, 0x21, 0x44, 0xb1,
	                             0x3f, 0x36, 0xe3, 0x8a, 0xc6, 0xd3, 0x9f, 0x95,
	                             0x88, 0x93, 0x93, 0x69, 0x28, 0x60, 0xb5, 0x1a,
	                             0x42 ,0xfb, 0x81, 0xef, 0x4d, 0xf7, 0xc5, 0xb8};
	SM2_SIGNATURE_STRUCT sm2_sig;
	unsigned char std_r[32] = {0xf5, 0xa0, 0x3b, 0x06, 0x48, 0xd2, 0xc4 ,0x63,
		                   0x0e, 0xea, 0xc5, 0x13, 0xe1, 0xbb, 0x81, 0xa1,
				   0x59, 0x44, 0xda, 0x38, 0x27, 0xd5, 0xb7, 0x41,
				   0x43, 0xac, 0x7e, 0xac, 0xee, 0xe7, 0x20, 0xb3};
	unsigned char std_s[32] = {0xb1, 0xb6, 0xaa, 0x29, 0xdf, 0x21, 0x2f, 0xd8,
		                   0x76, 0x31, 0x82, 0xbc, 0x0d, 0x42, 0x1c, 0xa1,
				   0xbb, 0x90, 0x38, 0xfd, 0x1f, 0x7f, 0x42, 0xd4,
				   0x84, 0x0b, 0x69, 0xc4, 0x85, 0xbb, 0xc1, 0xaa};
	int i;

	if ( error_code = sm2_sign_data_test(msg,
		                             msg_len,
					     user_id,
					     user_id_len,
					     pub_key,
					     pri_key,
					     &sm2_sig) )
	{
	   printf("Create SM2 signature by using input defined in standard failed!\n");
	   return error_code;
	}

	if ( memcmp(sm2_sig.r_coordinate, std_r, sizeof(std_r)) )
	{
	   printf("r coordinate of SM2 signature is invalid!\n");
	   return (-1);
	}
	if ( memcmp(sm2_sig.s_coordinate, std_s, sizeof(std_s)) )
	{
	   printf("s coordinate of SM2 signature is invalid!\n");
	   return (-1);
	}

	printf("Create SM2 signature by using input defined in standard succeeded!\n");
	printf("SM2 signature:\n");
	printf("r coordinate:\n");
	for (i = 0; i < sizeof(sm2_sig.r_coordinate); i++)
	{
	   printf("0x%x  ", sm2_sig.r_coordinate[i]);
	}
	printf("\n");
	printf("s coordinate:\n");
	for (i = 0; i < sizeof(sm2_sig.s_coordinate); i++)
	{
	   printf("0x%x  ", sm2_sig.s_coordinate[i]);
	}
	printf("\n\n");

	if ( error_code = sm2_verify_sig(msg,
		                         msg_len,
					 user_id,
					 user_id_len,
					 pub_key,
					 &sm2_sig) )
	{
	   printf("Verify SM2 signature created by using input defined in standard failed!\n");
	   return error_code;
	}
	printf("Verify SM2 signature created by using input defined in standard succeeded!\n");

	return 0;
}

unsigned char sm2_pri_key[32] = {0x65, 0x8F, 0x62, 0x0F, 0xCA, 0x76, 0x71, 0x1D, 0x5F, 0x86, 0x08, 0x03, 0xA2, 0x9B, 0x6A, 0x4D,
								 0x7C, 0x3D, 0xA4, 0x76, 0xB1, 0xFE, 0x3E, 0x8F, 0xE0, 0x6D, 0x8B, 0x9B, 0x86, 0xB3, 0x38, 0x94};


unsigned char sm2_pub_key[65] = {0x04, 0xb7, 0xa3, 0xf1, 0xaf, 0x1b, 0xf0, 0x36, 0x87, 
    								   0x3e, 0x45, 0x99, 0xef, 0xf2, 0xed, 0xa1, 0x76,
    								   0xb5, 0x37, 0x2f, 0x7d, 0x76, 0x22, 0xf1, 0x8f,
    								   0xd6, 0xaa, 0x56, 0x7d, 0x57, 0xa5, 0xa7, 0xf6,
    								   0x63, 0x8e, 0x27, 0xce, 0xb6, 0xe2, 0x33, 0x47,
    								   0xc4, 0xfa, 0x12, 0x4f, 0x35, 0x36, 0xe8, 0xcf,
    								   0x79, 0x26, 0xda, 0x6a, 0x5d, 0xc9, 0xb0, 0x94,
    								   0x3e, 0x54, 0x03, 0x06, 0x97, 0xbe, 0x0c, 0xf3};


unsigned char message[32] =   { 0x26, 0x77, 0xf8, 0x27, 0x46, 0x84, 0xaa, 0xa6,
							  0xf4, 0x9a, 0xe8, 0x72, 0x76, 0xf4, 0xbc, 0x9e,
							  0xf8, 0xd9, 0x5d, 0x0e, 0x5d, 0xeb, 0x1c, 0xe4,
							  0x56, 0xfe, 0xc9, 0x03, 0x17, 0xbb, 0x55, 0x38};

unsigned char sm2_signdata[64] =   { 0x60, 0x22, 0xcd, 0xf0, 0xfb, 0x3f, 0xe0, 0xaf,
									 0x2b, 0xcb, 0x30, 0x56, 0x0d, 0x3d, 0x46, 0xea,
									 0xb3, 0x7e, 0x22, 0x62, 0xd3, 0x42, 0x19, 0x10, 
									 0x64, 0x87, 0xde, 0x17, 0xe2, 0x5d, 0x52, 0xb0,
									 0x53, 0xed, 0x4a, 0x7b, 0x04, 0x15, 0x39, 0x5f,
									 0xab, 0x8f, 0x90, 0x73, 0x13, 0xa1, 0xfd, 0xf6,
									 0x2e, 0x00, 0x59, 0x0c, 0xe6, 0x95, 0x2f, 0x5d,
									 0xba, 0xe8, 0x07, 0x39, 0xe4, 0x7f, 0x72, 0x69};


/*********************************************************/
int test_sm2_sign_and_verify(void)
{
	int error_code;
	unsigned char msg[] = {"message digest"};
	unsigned int msg_len = (unsigned int)(strlen((char *)msg));
	unsigned char user_id[] = {"1234567812345678"};
	unsigned int user_id_len = (unsigned int)(strlen((char *)user_id));
	SM2_KEY_PAIR key_pair;
	SM2_SIGNATURE_STRUCT sm2_sig;
	int i;

	if ( error_code = sm2_create_key_pair(&key_pair) )
	{
	   printf("Create SM2 key pair failed!\n");
	   return (-1);
	}

	memcpy(key_pair.pri_key, sm2_pri_key, sizeof(sm2_pri_key));
	memcpy(key_pair.pub_key, sm2_pub_key, sizeof(sm2_pub_key));

	printf("Create SM2 key pair succeeded!\n");
	printf("Private key:\n");
	for (i = 0; i < sizeof(key_pair.pri_key); i++)
	{
	   printf("0x%02x  ", key_pair.pri_key[i]);
	}
	printf("\n\n");
	printf("Public key:\n");
	for (i = 0; i < sizeof(key_pair.pub_key); i++)
	{
	   printf("0x%02x  ", key_pair.pub_key[i]);
	}
	printf("\n\n");

	printf("/*********************************************************/\n");
	if ( error_code = sm2_sign_data(message,
		                        sizeof(message),
					user_id,
					user_id_len,
					key_pair.pub_key,
					key_pair.pri_key,
					&sm2_sig) )
	{
	   printf("Create SM2 signature failed!\n");
	   return error_code;
	}

	memcpy(&sm2_sig.r_coordinate[0], sm2_signdata, sizeof(sm2_signdata));
	printf("Create SM2 signature succeeded!\n");
	printf("SM2 signature:\n");
	printf("r coordinate:\n");
	for (i = 0; i < sizeof(sm2_sig.r_coordinate); i++)
	{
	   printf("0x%x  ", sm2_sig.r_coordinate[i]);
	}
	printf("\n");
	printf("s coordinate:\n");
	for (i = 0; i < sizeof(sm2_sig.s_coordinate); i++)
	{
	   printf("0x%x  ", sm2_sig.s_coordinate[i]);
	}
	printf("\n\n");

	if ( error_code = sm2_verify_sig(message,
		                         sizeof(message),
					 user_id,
					 user_id_len,
					 key_pair.pub_key,
					 (SM2_SIGNATURE_STRUCT *)&sm2_signdata ))
	{
	   printf("Verify SM2 signature failed!\n");
	   return error_code;
	}
	printf("Verify SM2 signature succeeded!\n");

	return 0;
}
