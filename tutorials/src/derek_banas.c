

// single line comment

/* Multi
Line
comment
*/

// import libraries
#include <stdio.h>
#include <string.h> // needed for string functions like strcpy(), strcat(), strcmp(), strlen()
#include <stdlib.h> // needed for exit() function (synonomous to sys.exit() in Python), and malloc()
#include <stdbool.h> // needed for bool type
#include <ctype.h> // needed for: tolower(), toupper(), isalnum() functions
#include <errno.h> // needed for error handling in tutorial12
#include <math.h> // needed for math functions in tutorial14 (requires "-lm" flag in gcc cmd)

// define constant
#define MY_NAME "Luke Dickerson"
int global_var = 100; // global variables are defined outside of main
// NOTE: These exist for the entire program lifetime. Stored in the data segment, not on the stack, and any function can access them

void variable_types_and_console_io();
void variable_types_and_console_io() {

	printf("\n\n==================== VARIABLE TYPES AND CONSOLE I/O ====================\n");

	printf("\nHello World!\n\n");
	printf("\ttab, 'single quote', \"double quote\", \\backslash\\, \nnew line\n\n");

	char a = 'a';
	int b = 48;
	long int c = -327670000;
	float d = 3.1236890;
	double e = 4.143256456756785;

	printf("character = '%c'\n", a);
	printf("int = %d\n", b);
	printf("long int = %ld\n", c);
	printf("float = %.5f\n", d);
	printf("double = %.5f\n", e);

	printf("MY_NAME = constant = %s\n", MY_NAME);

	char s1[10] = "hello";  // Creates a 10-character array, initializes with "hello\0"
	char s2[] = "auto sized character array";

	printf("s1 = %s\n", s1);
	strcpy(s1, "new value"); // strcpy is from string.h, and can be used to update the value of a string 
	printf("s1 = %s\n", s1);
	printf("s2 = %s\n", s2);

	// user input
	char c2;
	printf("user input: Enter char value: ");
	scanf("%c", &c2);
	printf("c2 = %c\n", c2);

	char middleInitial;
	char firstName[30], lastName[30];
	printf("what is your middle initial? ");
	scanf(" %c", &middleInitial);
	printf("what is your first and last name? ");
	scanf(" %s %s", firstName, lastName);
	printf("your name is %s %c %s\n", firstName, middleInitial, lastName);

	int month, day, year;
	printf("Enter your birth date (mm/dd/yyyy): ");
	scanf("%d/%d/%d", &month, &day, &year);
	printf("you were born on %02d/%02d/%04d\n", month, day, year);

}
void math();
void math() {
	int i1 = 12, i2 = 15;
	float f1 = 1.2, f2 = 1.5;
	printf("\n\n==================== MATH OPERATIONS ====================\n");
	printf("integer addition:       i1 + i2 = %d + %d = %d\n", i1, i2, i1 + i2);
	printf("integer subtraction:    i1 - i2 = %d - %d = %d\n", i1, i2, i1 - i2);
	printf("integer multiplication: i1 * i2 = %d * %d = %d\n", i1, i2, i1 * i2);
	printf("integer division:       i1 / i2 = %d / %d = %d\n", i1, i2, i1 / i2);
	printf("integer modulus:        i1 %% i2 = %d %% %d = %d\n", i1, i2, i1 % i2);
	printf("double addition:        f1 + f2 = %.1f + %.1f = %.1f\n", f1, f2, f1 + f2);
	printf("double subtraction:     f1 - f2 = %.1f - %.1f = %.1f\n", f1, f2, f1 - f2);
	printf("double multiplication:  f1 * f2 = %.1f * %.1f = %.1f\n", f1, f2, f1 * f2);
	printf("double division:        f1 / f2 = %.1f / %.1f = %.1f\n", f1, f2, f1 / f2);
	printf("double modulus:        N/A\n");

	printf("math without parantheses: 3 + 6 * 10 = %d\n", 3 + 6 * 10);
	printf("math with parantheses: (3 + 6) * 10 = %d\n", (3 + 6) * 10);
	// order of operations: parentheses, negative sign, not symbol, increment/decrement, multiplication/division/modulus, addition, subtraction, relational operators

	int i4 = 3;
	printf("i4 = %d\n", i4);
	printf("pre-increment ++i4 = %d\n", ++i4); // increments i4, then returns the value
	printf("post-increment i4++ = %d\n", i4++); // returns the value, then increments i4
	printf("pre-decrement --i4 = %d\n", --i4); // decrements i4, then returns the value
	printf("post-decrement i4-- = %d\n", i4--); // returns the value, then decrements i4
	printf("i4 after post-increment = %d\n", i4);
	printf("i4 += 5  --> i4 = i4 + 5 --> %d\n", i4 += 5);
	printf("i4 *= 2  --> i4 = i4 * 2 --> %d\n", i4 *= 2);
	printf("i4 /= 4  --> i4 = i4 / 4 --> %d\n", i4 /= 4);
	printf("i4 -= 3  --> i4 = i4 - 3 --> %d\n", i4 -= 3);
	printf("i4 %%= 2 --> i4 = i4 %% 2 --> %d\n", i4 %= 2);

	printf("cast int to float: (float) i1 / i2 = %.2f\n", (float) i1 / i2);

}
void tutorial1();
void tutorial1() {
	// source: https://www.youtube.com/watch?v=nXvy5900m3M
	variable_types_and_console_io();
	math();
}

void comparison_and_relational_operators();
void comparison_and_relational_operators() {

	printf("\n\n==================== COMPARISON & RELATIONAL OPERATORS ====================\n");

	// comparison operators: >, <, >=, <=, ==, !=
	int n1 = 1, n2 = 2;
	printf("Is n1 > n2? %d\n", n1 > n2);
	if (n1 > n2) {
		printf("n1 is greater than n2\n");
	} else if (n1 < n2) {
		printf("n1 is less than n2\n");
	} else {
		printf("n1 is equal to n2\n");
	}
	int customer_age = 38;
	if (customer_age > 21 && customer_age < 35) printf("customer is welcome\n");
	else printf("customer is not welcome\n");

	// logical operators ! (NOT), && (AND), || (OR)
	printf("! turns a true(1) to false(0) and vice versa: !0 = %d, !1 = %d\n", !0, !1);
	int bobMissedDays = 8, bobTotalSales = 24000,
		bobNewCustomers = 32;
	if (bobMissedDays < 10 && (bobTotalSales > 30000 || bobNewCustomers > 30)) {
		printf("Bob gets a raise!\n");
	} else {
		printf("Bob does not get a raise.\n");
	}

	// ternary operator
	customer_age = 32;
	char* legal_age = (customer_age >= 21) ? "true" : "false";
	printf("Is customer of legal age? %s\n", legal_age);
	int num_products = 10;
	printf("I bought %s product%s\n", (num_products > 1) ? "many" : "one");
}
void variable_sizes();
void variable_sizes() {
	printf("\n\n==================== VARIABLE SIZES ====================\n");	
	printf("a char   takes up %d byte(s)\n", sizeof(char));
	printf("a int    takes up %d byte(s)\n", sizeof(int));
	printf("a long   takes up %d byte(s)\n", sizeof(long));
	printf("a float  takes up %d byte(s)\n", sizeof(float));
	printf("a double takes up %d byte(s)\n", sizeof(double));
	/*
		"bit" is short for "binary digit"
		1 byte = 8 bits
	*/
}
void loops();
void loops() {
	printf("\n\n==================== LOOPS ====================\n");	
	// for loop
	printf("for loop: odd numbers\n");
	for (int i = 1; i <= 5; i++) {
		if (i % 2 == 0) continue; // skip even numbers
		printf("i = %d\n", i);
	}
	// while loop
	printf("\nwhile loop:\n");
	int j = 1;
	while (j <= 5) {
		printf("j = %d\n", j);
		j++;
	}
	// do-while loop
	printf("\ndo-while loop:\n");
	int k = 1;
	do {
		printf("k = %d\n", k);
		k++;
	} while (k <= 5);
}
void tutorial2();
void tutorial2() {
	// source: https://www.youtube.com/watch?v=6uIc4PtB9BM
	comparison_and_relational_operators();
	variable_sizes();
	loops();
}

int add(int a, int b); // Function Declaration
int add(int a, int b) { // Function Definition
	return a + b;
}
/* NOTE:
   Functions in C must be declared before they are used/called.
   This can be done by placing the function definition above main(),
   or by providing a function prototype (declaration) above main()
   and the function definition below main().

   If function f1 calss f2 does f2 have to be defined and implemented
   before f1? A definition is only required when a function is used
   before it is defined.
*/
void local_vs_global_variables();
void local_vs_global_variables() {
	int local_var = 40; // new local_var for this function
	printf("local_var inside of function: %d\n", local_var);
	global_var += 10;
	printf("global_var inside of function: %d\n", global_var); 
}
void tutorial3();
void tutorial3() {
	// source: https://www.youtube.com/watch?v=IBr78sxWN2M
	// In this tutorial I cover : exit(), switch, Arrays, Array Indexes, Problems with scanf(), Memory Overflow, strcpy(), fgets(), Array Interation, strcmp(), strcat(), strlen(), strlcpy(), Global Variables, Local Variables, Functions, and more

	printf("\n\n==================== LOOPS & SWITCHES & STRING FUNCTIONS ====================\n");

	// exit() function & switch statement
	int x = 3;
	switch(x) {
		case 1:
			printf("x is 1\n");
			break;
		case 2:
			printf("x is 2\n");
			break;
		case 3:
			printf("x is 3\n");
			break;
		default:
			printf("x is something else\n");
			exit(0); // exits the program immediately with exit code 0 (success)
			break;
	}

	// arrays
	char whole_name[12] = "Derek Banas"; // preallocated number of array elements
	int prime_numbers[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29}; // auto sized array
	printf("the first prime number is %d\n", prime_numbers[0]);

	// string functions
	// fgets (user input for strings)
	// problems with scanf():
	//     it stops reading input at the first whitespace character (space, tab, newline)
	//     it does not perform bounds checking, which can lead to write beyond the allocated memory for the array (buffer overflow)
	char your_city[30];
	printf("what city do you live in? ");
	fgets(your_city, 30, stdin); // args (string (aka char array) variable, length of string, input source)
	printf("Hello from %s", your_city); // fgets includes the newline character in the string
	for (int i = 0; i < strlen(your_city); i++) {
		if (your_city[i] == '\n') {
			your_city[i] = '\0'; // replace newline character with null terminator
			break;
		}
	}
	printf("Hello from %s.\n", your_city); // fgets includes the newline character in the string
	// string comparison, strcmp returns a negative number if str1 < str2, zero if str1 == str2, positive number if str1 > str2
	printf("is your city Paris? %d\n", strcmp(your_city, "Paris"));
	// string concatenation
	char greeting[50] = "Hello ";
	strcat(greeting, your_city);
	printf("%s\n", greeting);
	// string length
	printf("the length of the greeting is %d\n", strlen(greeting));
	// strlcpy is better than strcpy (string copy) because it can overwrite other parts of memory if the source string is too long
	strlcpy(your_city, "A very very long city name that exceeds the buffer", sizeof(your_city)); // args: (destination, source, size of destination)
	printf("New City: %s\n", your_city);

	// functions
	int sum = add(5, 10);
	printf("The sum of 5 and 10 is %d\n", sum);

	int local_var = 30;
	printf("local_var before function call: %d\n", local_var);
	printf("global_var before function call: %d\n", global_var); 
	local_vs_global_variables();
	printf("global_var after function call: %d\n", global_var); 

}

void passIntPointersAndChangeValue(int* num1, int* num2) {
	// generate two random numbers between 1 and 50
	*num1 = rand() % 50 + 1; // rand() generates a random number, % 50 gives a number between 0 and 49, + 1 gives a number between 1 and 50
	*num2 = rand() % 50 + 1;
	printf("Inside function:      num1 = %d, num2 = %d\n", *num1, *num2);
}
void passStringPointersAndChangeValue(char* msg, int size) {
	char newMessage[] = "Goodbye";
	if (size > sizeof(newMessage)) {
		for (int i = 0; i < sizeof(newMessage); i++) {
			msg[i] = newMessage[i];
		}
	} else {
		printf("New Message is too big");
	}
}
void tutorial4();
void tutorial4() {
	// source: https://www.youtube.com/watch?v=N7MRxREJ4-U
	// pointers, arrays, and functions

	// pointers (aka memory addresses of variables)
	// note: often variables declared together have memory addresses right after eachother but sometimes not
	int n1 = 12, n2 = 15; // each int uses 4 bytes
	printf("size of n1 = %d, size of n2 = %d\n", sizeof(n1), sizeof(n2));
	// The '&' symbol before a variable name returns the memory address of that variable.
	// It's called the address-of operator.
	printf("n1 memory address = %p, n2 memory address = %p\n", &n1, &n2);
	// To pass a pointer to a function, store the pointer itself in its own variable using
	// the * operator after the data type. The pointer's data type must be the same as the
	// variable it points to (with the exception: void* is a generic pointer type, it can
	// hold the address of any data type).
	int* p1 = &n1; // p1 is a pointer to an int, initialized to the address of n1
	int* p2 = &n2;
	// so now when printing you don't need the '&' symbol
	printf("p1 =                %p, p2 =                %p\n", p1, p2);
	// To get the value stored at the memory address (aka in the pointer)
	// use '*' before the pointer variable name. It's called the dereference operator.
	printf("p1 value = %d, p2 value = %d\n", *p1, *p2);
	// it can be used for assignment too:
	*p1 = 13;
	printf("After changing *p1, n1 = %d\n", n1);
	// so in short: '&' gets the memory address of a variable, and '*' gets the value at a pointer
	// one way to memorize it is the '&' looks like an 'A' for address
	// now for arrays we can use pointers to access array elements (aka pointer arithmetic)
	int prime_numbers[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29}; // int array
	printf("array's 1st index's value: %d\n", prime_numbers[0]);
	printf("array's 1st index's value: %d\n", *prime_numbers);
	printf("array's 2nd index's value: %d\n", prime_numbers[1]);
	printf("array's 2nd index's value: %d\n", *(prime_numbers+1)); // pointer arithmetic
	char* students[] = {"Alice", "Bob", "Charlie"}; // string array
	for (int i = 0; i < 3; i++) {
		printf("address %p, student: %s\n", &students[i], students[i]);
	}
	// functions with pointers
	int num1 = 0, num2 = 0;
	printf("Before function call: num1 = %d, num2 = %d\n", num1, num2);
	passIntPointersAndChangeValue(&num1, &num2);
	printf("After function call:  num1 = %d, num2 = %d\n", num1, num2);
	char message[10] = "Hello";
	printf("Orignal Message: %s\n", message);
	passStringPointersAndChangeValue(message, sizeof(message));
	printf("New Message:     %s\n", message);
}

struct dogsFavs {
	const char* food;
	const char* friend;
};
typedef struct {
	const char* name;
	const char* breed;
	int avg_height_cm;
	int avg_weight_lbs;
	struct dogsFavs favoriteThings;
} dog;
void getDogInfo(dog d) {
	printf("Dog Name: %s\n", d.name);
	printf("Dog Breed: %s\n", d.breed);
	printf("Average Height (cm): %d\n", d.avg_height_cm);
	printf("Average Weight (lbs): %d\n", d.avg_weight_lbs);
}
void getMemoryLocation(dog d) {
	printf("Name   Address Location: %p\n", d.name);
	printf("Breed  Address Location: %p\n", d.breed);
	printf("Height Address Location: %p\n", &d.avg_height_cm);
	printf("Weight Address Location: %p\n", &d.avg_weight_lbs);
}
void getDogsFavs(dog d) {
	printf("%s loves %s and his friend is %s.\n",
		d.name, d.favoriteThings.food, d.favoriteThings.friend);
}
void setDogWeight(dog d, int newWeight) {
	d.avg_weight_lbs = newWeight; // this will not change the original dog's weight because structs are passed by value
	printf("Inside setDogWeight function, new weight: %d\n", d.avg_weight_lbs);
}
void setDogWeightPtr(dog* d, int newWeight) {
	(*d).avg_weight_lbs = newWeight; // this will change the original dog's weight because we are using a pointer
	d->avg_weight_lbs = newWeight; // this is another way to change/read the data in a struct's pointer
	printf("Inside setDogWeightPtr function, new weight: %d\n", d->avg_weight_lbs);
}
void tutorial5();
void tutorial5() {
	// source: https://www.youtube.com/watch?v=zv8OdxQuUl8
	// structs
	
	// structs are used when you need more than one piece of data to describe one thing
	dog cujo = {"Cujo", "Saint Bernard", 90, 264, {"Meat", "Joe Camp"}};
	getDogInfo(cujo);
	dog cujo2 = cujo;
	getMemoryLocation(cujo);
	getMemoryLocation(cujo2);
	getDogsFavs(cujo);
	setDogWeight(cujo, 270); // this will not change cujo's weight
	printf("Outside setDogWeight function, cujo's weight: %d\n", cujo.avg_weight_lbs);
	setDogWeightPtr(&cujo, 270); // this will change cujo's weight
	printf("Outside setDogWeightPtr function, cujo's weight: %d\n", cujo.avg_weight_lbs);

}

typedef struct product { // "product" is at the top as well so it can be used recursively
	const char* name;
	float price;
	struct product* next; // pointer to the next product in the linked list
} product;
void printLinkedList(product* pProduct) {
	while (pProduct != NULL) {
		printf("A %s costs $%.2f\n", (*pProduct).name, pProduct->price);
		pProduct = pProduct->next; // move to the next product in the list
	}
}
void tutorial6();
void tutorial6() {
	// source: https://www.youtube.com/watch?v=MjQ6PEQen-Q
	// stucts, unions, enums, linked lists, and recursive structures

	// Unions allow you to store data of different types in one variable.
	// The size of the union is the size of its largest member.
	// Only one value is stored at a time, the value that was last written to.

	// Example: Many ways to sell oranges:
	// Price Per Orange: $0.45
	// Price Per Pound: $1.14
	// Price Per Ounce of Orange Juice: $2.43
	typedef union {
		short int individual;
		float pound;
		float ounce;
	} amount;
	// this union stores the amount of oranges a customer wants to buy, in the 3 different units the seller offers the sale in, so the program could multiply by the correct price to get the total price.

	// to add values to it:
	amount orangeAmt = {.ounce = 16}; // designated initializers (C99 and later)
	orangeAmt.individual = 4; // a way to update it (you could also just reset it with "orangeAmt = {.individual = 4};")
	printf("%d oranges, Memory Size: %d byte(s).\n", orangeAmt.individual, sizeof(orangeAmt.individual));
	printf("%.2f pounds of oranges, Memory Size: %d byte(s).\n", orangeAmt.ounce, sizeof(orangeAmt.ounce));
	orangeAmt.pound = 1.5;
	printf("%.2f ounces of orange juice, Memory Size: %d byte(s).\n", orangeAmt.pound, sizeof(orangeAmt.pound));

	// using a union inside a struct
	typedef struct {
		char brand[20];
		amount theAmount;
	} orangeProduct;
	orangeProduct productOrdered = {"Chiquita", .theAmount.ounce = 16.0};
	printf("You bought %.2f ounce(s) of %s oranges.\n", productOrdered.theAmount.ounce, productOrdered.brand);

	// enumerated types (enums)
	typedef enum { INDIV, OZ, LBS } quantity;
	quantity quantityType = INDIV;
	orangeAmt.individual = 4;
	if (quantityType == INDIV) {
		printf("You ordered %d individual oranges.\n", orangeAmt.individual);
	} else if (quantityType == OZ) {
		printf("You ordered %.2f ounces of orange juice.\n", orangeAmt.ounce);
	} else if (quantityType == LBS) {
		printf("You ordered %.2f pounds of oranges.\n", orangeAmt.pound);
	}

	// recursive structures and linked lists
	product tomato = {"Tomato", 0.51, NULL};
	product potato = {"Potato", 1.21, NULL};
	product lemon  = {"Lemon",  1.69, NULL};
	product milk   = {"Milk",   3.09, NULL};
	product turkey = {"Turkey", 1.89, NULL};
	tomato.next = &potato;
	potato.next = &lemon;
	lemon.next  = &milk;
	milk.next   = &turkey;
	product apple = {"Apple", 1.58, NULL};
	lemon.next = &apple; // inserting apple between lemon and milk
	apple.next = &milk;
	printLinkedList(&tomato);

}

void to_lowercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]); // convert each character
    }
}
void to_uppercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]); // convert each character
    }
}
void getCharInfo(char c) {
	while ((c = getchar()) != '\n') {
		printf("Character: '%c', ASCII Value: %d\n", c, (int)c);
	}
}
void tutorial7();
void tutorial7() {
	// source: https://www.youtube.com/watch?v=FqpbxjiaB5o
	// messing with strings, chars, booleans, functions & pointers

	// boolean type
	_Bool isSunny = 0; // _Bool is a built-in type in C99 and later, its values are 0 (false) and 1 (true)
	bool isANumber = true; // "bool" is an alias for _Bool, and "true" is 1, "false" 0. They are defined in <stdbool.h>, its more readable
	int number;
	int sumOfNumbers = 0;
	printf("Enter a number:\n");
	isANumber = (scanf("%d", &number) == 1); // scanf returns the number of successfully read items
	while (isANumber) {
		sumOfNumbers += number;
		isANumber = (scanf("%d", &number) == 1); // scanf returns the number of successfully read items
	}
	printf("The sum of the numbers you entered is: %d\n", sumOfNumbers);

	// getChar() reads user input, and putChar() outputs a character
	printf("enter ~ to exit while loop:\n");
	char theChar;
	while ((theChar = getchar()) != '~') {
		putchar(theChar);
	}

	// .. i skipped Derek's section on gets, puts, fgets, and fputs

	// strchr and strrchr
	char* theString = "This is a sample string.";
	printf("The string is: \"%s\"\n", theString);
	char* firstS = strchr(theString, 's'); // return a pointer to the first occurence of a character in the string, or NULL if not found
	char* lastS = strrchr(theString, 's'); // return a pointer to the last occurence of a character in the string, or NULL if not found

    if(firstS != NULL) {
        printf("First 's' at position: %ld\n", firstS - theString);
    } else {
        printf("'s' not found\n");
    }
    if(lastS) {
        printf("Last 's' at position: %ld\n", lastS - theString);
    } else {
        printf("'s' not found\n");
    }

	// tolower() and toupper()
	char text[] = "Hello, World! 123";
    printf("Original string: %s\n", text);
    to_lowercase(text);
    printf("Lowercase string: %s\n", text);
    to_uppercase(text);
    printf("Uppercase string: %s\n", text);

	// isalnum() is a function that checks whether a character is
	// alphanumeric (i.e., a letter A-Z/a-z or a digit 0-9). It
	// returns a non-zero value (true) if the character is alphanumeric,
	// or 0 (false) otherwise.
	// isalpha() checks if a character is a letter A-Z/a-z
	// isblank() checks if a character is a space or tab
	// isgraph() checks if a character has a graphical representation (basically anything but space)
	// iscntrl() checks if a character is a control character
	// isdigit() checks if a character is a digit 0-9
	// ispunct() checks if a character is a punctuation character
	// isspace() checks if a character is a whitespace character (space, tab, or newline)
    char str[] = "Hello123!@#";
    printf("Alphanumeric characters in '%s':\n", str);
    for (int i = 0; str[i]; i++) {
        if (isalnum((unsigned char)str[i])) {
            printf("%c ", str[i]);
        }
    }
    printf("\n");
}

typedef struct {
    char name[50];
    int age;
} Person;
void tutorial8() {
	// source: https://www.youtube.com/watch?v=PYzqi0eOxIg
	// how to dynamically allocation memory with malloc() (aka memory allocator function)

	int amountOfNumbersToStore;
	printf("how many integers do you want to store? ");
	scanf("%d", &amountOfNumbersToStore);
	int* pRandomNumbers = (int *) malloc(amountOfNumbersToStore * sizeof(int)); // malloc allocates memory at runtime (dynamic memory allocation). at runtime, pass the ammount of memory you want to create (in bytes) as an argument to malloc, and it returns a pointer to the first byte of the allocated memory block. if the allocation fails, it returns NULL.
	if (pRandomNumbers == NULL) {
		printf("Memory allocation failed!\n");
		exit(1); // exit the program with an error code
	} else {
		printf("%d byte(s) of memory allocated\n", amountOfNumbersToStore * sizeof(int));
	}
	free(pRandomNumbers); // free releases the memory allocated by malloc back to the system. always free memory when you are done using it to avoid memory leaks.

	// dynamically allocate memory for an array of structs
    // Dynamically allocate memory for one Person
    Person *p = (Person *)malloc(sizeof(Person));
    if (p == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(1); // exit the program with an error code
    }

    // Set values
    strcpy(p->name, "Alice");
    p->age = 30;

    // Access and print values
    printf("Name: %s\n", p->name);
    printf("Age: %d\n", p->age);

    // Free the memory
    free(p);

    // Dynamically allocate memory for an array of Persons
    int n = 3;
    Person *people = (Person *)malloc(n * sizeof(Person));
    if (people == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(1); // exit the program with an error code
    }
    // Set values for each person
    strcpy(people[0].name, "Bob");
    people[0].age = 25;
    strcpy(people[1].name, "Carol");
    people[1].age = 28;
    strcpy(people[2].name, "Dave");
    people[2].age = 35;
    // Print each person
    for (int i = 0; i < n; i++) {
        printf("Person %d: %s, %d\n", i+1, people[i].name, people[i].age);
    }
    // Free the memory for the array
    free(people);

}

void tutorial9() {
	// source: https://www.youtube.com/watch?v=muco_oVlNwQ
	// working with struct linked lists

	// skipped for now
}

void tutorial10() {
	// source: https://www.youtube.com/watch?v=muco_oVlNwQ
	// continued ... working with struct linked lists: search, delete, and free memory of struc tlinked lists

	// skipped for now
}

void tutorial11() {
	// source: https://www.youtube.com/watch?v=FioQNaLA2zY
	// text file I/O in C

	// write to file
	FILE* pFile = fopen("example.txt", "w"); // "w" opens/creates file for writing
	// "a" appends to the end of the file
	// "r" opens file for reading
	// "r+" opens file for reading and writing (does not create file if it doesn't exist)
	// "w+" opens/creates file for reading and writing (overwrites existing file)
	if (!pFile) {
		printf("Error opening file for writing.\n");
		exit(1); // exit the program with an error code
	}
	fprintf(pFile, "Hello, World! %d\n", 777);
	if (fclose(pFile) != 0) {
		printf("Error closing file after writing.\n");
		exit(1); // exit the program with an error code
	}
	printf("File written successfully.\n");

	// read to file
	char buffer[1000];
	pFile = fopen("example.txt", "r");
	if (!pFile) {
		printf("Error opening file for reading.\n");
		exit(1); // exit the program with an error code
	}
	while ( fgets(buffer, sizeof(buffer), pFile) != NULL ) {
		printf("Read from file: %s", buffer);
	}
	fseek(pFile, 0, SEEK_SET);
	// pFile = file pointer
	// 0 = offset
	// SEEK_SET = from the beginning of the file
	// NOTE: Unlike rewind, fseek() does not automatically clear error or EOF flags.
	while (fscanf(pFile, "%s", buffer) != EOF) {
		puts(("Read from file: %s\n", buffer));
	}
    rewind(pFile);
	// Reset file pointer to beginning
	printf("File read successfully.\n");
}

void tutorial12() {
	// source: https://www.youtube.com/watch?v=oSXIkgCSAoI
	// binary file I/O & error handling with C

	// test error handling
	FILE *fp;
	char *buffer;
	size_t dataInFile; // size_t is a data type used to represent the size of an element in bytes.
	long fileSize;
	fp = fopen("names.bin", "rb+"); // open file for reading/writing in binary mode
	if (fp == NULL) {
		perror("Error opening file"); // perror will print this string and put the error code in the variable errno from the errno.h library
		printf("Error code: %d\n", errno); // errno is a global variable that stores the error code of the last error that occurred

		// create file for writing/reading
		printf("file being created...\n");
		fp = fopen("names.bin", "wb+");
		if (fp == NULL) {
			perror("Error creating file");
			printf("Error code: %d\n", errno);
			exit(1); // exit the program with an error code
		}
	}

	// write to binary file
	char name[] = "Derek Banas";
	fwrite(
		name, // pointer to data to write
		sizeof(name[0]), // size of each element
		sizeof(name) / sizeof(name[0]), // number of elements to write
		fp // file pointer to write to
	); // write to binary file

	// get file size
	fseek(fp, 0, SEEK_END); // move file pointer to end of file
	fileSize = ftell(fp); // get current file pointer position (which is the size of the file)
	rewind(fp); // move file pointer back to beginning of file
	buffer = (char *) malloc(sizeof(char) * fileSize); // allocate memory for buffer to read file into
	if (buffer == NULL) {
		perror("Error allocating memory");
		printf("Error code: %d\n", errno);
		exit(2); // exit the program with an error code
	}

	// read from binary file
	dataInFile = fread(
		buffer, // pointer to buffer to read data into
		sizeof(char), // size of each element
		fileSize, // number of elements to read
		fp
	);
	if (dataInFile != fileSize) {
		perror("Error reading file");
		printf("Error code: %d\n", errno);
		exit(3); // exit the program with an error code
	}
	printf("Read from binary file: %s\n", buffer);
	fclose(fp);
	free(buffer); // free allocated memory

	// create and write an array to a binary file,
	// and read out any element inside of that file
	fp = fopen("numbers.bin", "wb+");
	int numbersToWrite[] = {10, 20, 30, 40, 50};
	fwrite(
		numbersToWrite,
		sizeof(int),
		sizeof(numbersToWrite) / sizeof(numbersToWrite[0]),
		fp
	);
	long indexToRead = 3; // read the 4th element (index 3)
	int numberRead;
	fseek(fp, indexToRead * sizeof(int), SEEK_SET); // move file pointer to the 4th element
	fread(
		&numberRead,
		sizeof(int),
		1,
		fp
	);
	printf("Read number at index %ld: %d\n", indexToRead, numberRead);
	fclose(fp);
	
}

char * convert_base(unsigned int n, int base) {
	char buffer[33];
	char * pConvertedNumber;
	char allValues[] = "0123456789abcdef";
	if (base < 2 || base > 16) {
		return NULL; // base not supported
	}
	pConvertedNumber = &buffer[sizeof(buffer) - 1]; // point to the end of the buffer
	pConvertedNumber[0] = '\0'; // set initial value to nothing
	do {
		int value = n % base;
		pConvertedNumber--; // move pointer back one position
		*pConvertedNumber = allValues[value]; // insert integer as character
		n /= base;
	} while (n != 0);
	return pConvertedNumber;
}
void tutorial13() {
	// source: https://www.youtube.com/watch?v=7F_2Jq4L_tA
	// converting from base 10 to base 2, 8, and 16

	unsigned int n1 = 60; // 111100 in binary
	printf("%d in binary is:  %s\n", n1, convert_base(n1, 2));
	printf("%d in base 8 is:  %s\n", n1, convert_base(n1, 8));
	printf("%d in base 16 is: %s\n", n1, convert_base(n1, 16));
}

int baseToDecimal(char * n, int baseFrom, int sizeOfNumber) {
	int result = 0;
	int toThePowerOf = 0;
	for (int i = sizeOfNumber - 2; i >= 0; --i) {
		if (isalpha(n[i])) {
			int charCode = ((int)tolower(n[i])) - 87;
			result += charCode * pow(baseFrom, toThePowerOf);
		} else {
			result += (n[i] - '0') * pow(baseFrom, toThePowerOf);
		}
		toThePowerOf++;
	}
	return result;
}
void tutorial14() {
	// source: https://www.youtube.com/watch?v=Ej8iYpTOrlE
	// converting from other bases to base 10
	char binaryNumber[] = "111100"; // binary for 60
	printf("%s in binary = %d in decimal\n", binaryNumber, baseToDecimal(binaryNumber, 2, sizeof(binaryNumber)));
	char base8Number[] = "74"; // binary for 60
	printf("%s in base 8 = %d in decimal\n", base8Number, baseToDecimal(base8Number, 8, sizeof(base8Number)));
	char hexidecimalNumber[] = "3C"; // binary for 60
	printf("%s in hexidecimal = %d in decimal\n", hexidecimalNumber, baseToDecimal(hexidecimalNumber, 16, sizeof(hexidecimalNumber)));
}

char * convertBase(unsigned int n, int base, char* pConvertedNumber) {
	char allValues[] = "0123456789abcdef";
	if (base < 2 || base > 16) {
		return NULL; // base not supported
	}
	char* p = pConvertedNumber + 32;
	*p = '\0'; // set end of string to null terminator
	do {
		int value = n % base;
		p--; // move pointer back one byte
		*p = allValues[value]; // insert integer as character
		n /= base;
	} while (n != 0);
	return p;
}
void tutorial15() {
	// source: https://www.youtube.com/watch?v=fqKiL03afUA
	// passing memory addresses, bitwise operators, signed integers,
	// shift operators, two's compliment, and bit masking
	
	// convert decimal to binary
	unsigned int n = 6;
	unsigned int m = 7;
	char *pConvertedNumber = malloc(33 * sizeof(char));
	printf("%d in binary is %s\n", n, convertBase(n, 2, pConvertedNumber));

	// bitwise operators
	/* boolean arithmatic
		AND operator (using '&' symbol):
			for each bit, return a 1 if BOTH input bits are 1, else 0
			example:
				6 in binary is 110
				7 in binary is 111
				110 & 111 = 110
		OR operator (using '|' symbol):
			for each bit, return a 1 if EITHER input bits are 1, else 0
			example:
				6 in binary is 110
				7 in binary is 111
				110 | 111 = 111
		EXCLUSIVE OR (aka XOR) operator (using '^' symbol):
			for each bit, return a 1 if ONE input bit is 1 and the other is 0, else 0
			example:
				6 in binary is 110
				7 in binary is 111
				110 | 111 = 001
		
	*/
	unsigned int andResult = n & m;
	printf("%s & ", convertBase(n, 2, pConvertedNumber));
	printf("%s = ", convertBase(m, 2, pConvertedNumber));
	printf("%s\n",  convertBase(andResult, 2, pConvertedNumber));
	unsigned int orResult = n | m;
	printf("%s | ", convertBase(n, 2, pConvertedNumber));
	printf("%s = ", convertBase(m, 2, pConvertedNumber));
	printf("%s\n",  convertBase(orResult, 2, pConvertedNumber));
	unsigned int xorResult = n ^ m;
	printf("%s ^ ", convertBase(n, 2, pConvertedNumber));
	printf("%s = ", convertBase(m, 2, pConvertedNumber));
	printf("%s\n",  convertBase(xorResult, 2, pConvertedNumber));

	// for signed integers, 2s compliment is used to not take up a but with the negative sign
	/*
		so C also has the ONES COMPLIMENT operator (using the '~' symbol):
			and this just flips all the bits the from 0 to 1, or 1 to 0
			its also called the bitwise NOT operator, which is different than regular boolean logical NOT operator (using the '!' symbol)
	*/
	unsigned int onesCompResult = ~n;
	printf("~%s ", convertBase(n, 2, pConvertedNumber));
	printf("%s\n",  convertBase(onesCompResult, 2, pConvertedNumber));
	printf("Negative of %d = %d\n", n, onesCompResult + 1);
	free(pConvertedNumber);

	// shift operators let you shift the bits to the left of the right by a specified number
	unsigned int shiftLeft2 = n << 2;
	printf("%s (aka the number %d) ", convertBase(n, 2, pConvertedNumber), n);
	printf("shifted 2 bits to the left = %s (the number %d)\n", 
		convertBase(shiftLeft2, 2, pConvertedNumber), shiftLeft2);
	unsigned int shiftRight2 = n >> 2;
	printf("%s (aka the number %d) ", convertBase(n, 2, pConvertedNumber), n);
	printf("shifted 2 bits to the right = %s (the number %d)\n", 
		convertBase(shiftRight2, 2, pConvertedNumber), shiftRight2);

	// bit masking
	/* bit masking is used to select parts of a series of bits
		example:
			x    = 11101010
			mask = 00001111
			x & mask returns just last 4 bits of x
	*/
	unsigned int x = 234;   // 234 = 11101010
	unsigned int mask = 15; // 15 = 00001111
	printf("x & mask = %s & ", convertBase(x, 2, pConvertedNumber));
	printf("%s = ", convertBase(mask, 2, pConvertedNumber));
	printf("%s\n", convertBase(x & mask, 2, pConvertedNumber));

}

// NOTE: all functions must be declared before they are used/called in main()
int main() {
	// tutorial1();
	// tutorial2();
	// tutorial3();
	// tutorial4();
	tutorial5();
	// tutorial6();
	// tutorial7();
	// tutorial8();
	// tutorial9();
	// tutorial10();
	// tutorial11();
	// tutorial12();
	// tutorial13();
	// tutorial14();
	// tutorial15();
	return 0;
}

