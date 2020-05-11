# TypeConversions from decimal and binary 
# to their respective octal representations 

# The choices present to the user 
print("a. Hexadecimal to Octal ") 
print("b. Decimal to Octal") 
print("c. Binary to Octal") 

# Function generates octal representation 
# from it's binary from 
def bin_to_oct(): 
	
	print("Enter your input in BIN format :-") 
	
	# taking user input as binary string and 
	# then using int() to convert it into it's 
	# respective decimal format 
	x = int(input(), 2) 
	print("Octal form of " + str(x) + " is " + oct(x) ) 


# Function generates octal representation 
# of it's hexadecimal form passed as value. 
def hex_to_oct(): 
	print("Enter your input in HEX format :-") 

	# taking user input as hexadecimal string and 
	# then using int() to convert it into it's 
	# respective decimal format 
	x = int(input(), 16) 
	print("Octal form of " + str(x) + " is " + oct(x)) 


# Function converts decimal form to it's 
# respective octal representation 
def decimal_to_oct(): 

	print("Enter a number with base-10 format :-") 

	# taking a simple user input and 
	# converting it to an integer 
	x = int(input()) 
	print("Octal form of " + str(x) + " is " + oct(x)) 


# Driver Code 
ch = input("Enter your choice :-\n") 

if ch is 'a': 
	hex_to_oct() 
elif ch is 'b': 
	decimal_to_oct() 
elif ch is 'c': 
	bin_to_oct() 

