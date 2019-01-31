'''
Script implementing caesar cipher

Usage: python3 caesar.py [action] [plaintext/ciphertext] [key]

action - e for encrypt, d for decrypt

e.g.- python3 caesar.py e "hello world" 2


Please Note: this implementation is only considering there will be alphabets in the plaintext or ciphertext.

Important functions, data structures and formulas:

1. sys.argv
2. enumerate
3. ord()
4. chr()
5. For Decryption, Formula:
	
	n = ord(CHAR)
	n = n - key
 	n = ord("Z") + 1 - ( ord("A") - n )  

'''

import sys

def main():

	action = sys.argv[1]

	if action == 'e' or action =='d':
		
		text = sys.argv[2]
	else:
		print("Error: Invalid action")
		exit()


	key = int(sys.argv[3]) % 26

	text = list(text)


	if action == 'e':	

		for i,j in enumerate(text):
	
			if (j >= "A" and j <= "Z") or (j >= "a" and j <= "z"):
			
				n = ord(j)
				n = n + key
				text[i] = chr(n)

	else:


		for i,j in enumerate(text):

			n = ord(j)
			
			n = n- key

			if (j >= "A" and j <= "Z"):

				if n  < 65:

					n = ord("Z") + 1 - ( ord("A") - n )

			elif (j >= "a" and j <= "z"):


				if n < 97:

					n = ord("z") + 1 - ( ord("a") - n)
			
			text[i] = chr(n)
				

	if action == "e":
		
		print("After Encryption, CipheText:","".join(text))

	else:

		print("After Decryption, PlainText:","".join(text))




main()
