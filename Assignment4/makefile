all: keygen otp_enc otp_enc_d otp_dec otp_dec_d


keygen: keygen.c
	gcc -g -Wall -std=c99 -o keygen  keygen.c

otp_enc: otp_enc.c
	gcc -g -Wall -std=c99 -o otp_enc otp_enc.c

otp_enc_d: otp_enc_d.c
	gcc -g -Wall -std=c99 -lpthread  -o otp_enc_d otp_enc_d.c

otp_dec: otp_dec.c
	gcc -g -Wall -std=c99 -o otp_dec otp_dec.c

otp_dec_d: otp_dec_d.c
	gcc -g -Wall -std=c99 -o otp_dec_d otp_dec_d.c

clean:
	rm -f keygen
	
