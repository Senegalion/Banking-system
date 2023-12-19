#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <string.h>

#define NUMBER_OF_OPERATIONS 8
#define MAX_LENGTH_OF_ACCOUNT_NUMBER 6
#define MAX_LENGTH_OF_NAME_AND_SURNAME 30
#define MAX_LENGTH_OF_ADDRESS 100
#define NUMBER_OF_DIGITS_IN_PESEL 11
#define MAXIMUM_LINE_LENGTH 9 + MAX_LENGTH_OF_ADDRESS
#define MAXIMUM_AMOUNT_OF_MONEY 10000
#define FILE_TRACK "banking_system.txt"
#define DEPOSIT_OPERATION 4
#define WITHDRAWAL_OPERATION 5
#define SUB_ACCOUNT_OPERATION 7

int is_exit = 1;
int can_transfer_to_sub_account;
int can_transfer_from_sub_account;

struct Bank_Account
{
    int account_number;
    char name[MAX_LENGTH_OF_NAME_AND_SURNAME + 1];
    char surname[MAX_LENGTH_OF_NAME_AND_SURNAME + 1];
    char address[MAX_LENGTH_OF_ADDRESS + 1];
    char pesel[NUMBER_OF_DIGITS_IN_PESEL + 1];
    int balance_regular_account;
    int balance_savings_sub_account;
};

void clear_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

void display_start_screen()
{
    printf("\nWelcome to Customer Bank Account Management System\n\n");
    printf("How can I help you?\n");
    printf("------------------------------------------------------------------------------------------------------------------------------\n");
    printf("| 1 - Create a new account with the following fields describing the customer                                                 |\n");
    printf("| 2 - List all accounts present in the database                                                                              |\n");
    printf("| 3 - Search for an account using one of the following fields: account number, name, surname, address, identification number |\n");
    printf("| 4 - Make a deposit                                                                                                         |\n");
    printf("| 5 - Make a withdrawal                                                                                                      |\n");
    printf("| 6 - Make a money transfer between two accounts                                                                             |\n");
    printf("| 7 - Make a money transfer to/from savings sub-account                                                                      |\n");
    printf("| 8 - Exit                                                                                                                   |\n");
    printf("------------------------------------------------------------------------------------------------------------------------------\n");
}

int ask_for_confirmation()
{
    char confirmation_answer;
    int is_correct_answer = 1;
    do
    {
        if (!is_correct_answer)
        {
            printf("Invalid answer.\n");
        }
        printf("Do you want to do this (Y/N)\n");
        clear_buffer();
        confirmation_answer = getchar();
        if (confirmation_answer == 'Y' || confirmation_answer == 'y' || confirmation_answer == 'N' || confirmation_answer == 'n')
        {
            is_correct_answer = 1;
        }
        else
        {
            is_correct_answer = 0;
        }
        clear_buffer();
    } while (!is_correct_answer);

    if (confirmation_answer == 'Y' || confirmation_answer == 'y')
    {
        return 1;
    }
    return 0;
}

int generate_account_number()
{
    int is_unique;
    int account_number;

    FILE *file = fopen(FILE_TRACK, "rb");
    if (file == NULL)
    {
        printf("Error opening file.\n");
        return -1;
    }

    do
    {
        srand(time(NULL));
        account_number = rand() % (int)((10 - 1) * pow(10, MAX_LENGTH_OF_ACCOUNT_NUMBER - 1)) + (int)pow(10, MAX_LENGTH_OF_ACCOUNT_NUMBER - 1);

        if (account_number < 0)
        {
            account_number *= (-1);
        }

        is_unique = 1;
        struct Bank_Account account;
        rewind(file);
        while (fread(&account, sizeof(struct Bank_Account), 1, file))
        {
            if (account.account_number == account_number)
            {
                is_unique = 0;
                break;
            }
        }
    } while (!is_unique);

    fclose(file);
    return account_number;
}

int is_valid_name_surname(const char *str, int max_length)
{
    if (strlen(str) >= max_length - 1)
    {
        clear_buffer();
        return 0;
    }

    char *newline = strchr(str, '\n');
    if (newline != NULL)
    {
        *newline = '\0';
    }

    int length = strlen(str);
    for (int i = 0; i < length; i++)
    {
        if (!isalpha(str[i]) || str[i] == ' ')
            return 0;
    }
    return 1;
}

int is_valid_address(const char *str, int max_length)
{
    if (strlen(str) >= max_length - 1)
    {
        clear_buffer();
        return 0;
    }

    char *newline = strchr(str, '\n');
    if (newline != NULL)
    {
        *newline = '\0';
    }
    return 1;
}

int is_valid_pesel(const char *pesel)
{
    int pesel_length = strlen(pesel);

    if (pesel_length != NUMBER_OF_DIGITS_IN_PESEL)
    {
        if (pesel_length > NUMBER_OF_DIGITS_IN_PESEL)
        {
            clear_buffer();
        }
        return 0;
    }

    char *newline = strchr(pesel, '\n');
    if (newline != NULL)
    {
        *newline = '\0';
    }

    for (int i = 0; i < NUMBER_OF_DIGITS_IN_PESEL; i++)
    {
        if (!isdigit(pesel[i]))
        {
            return 0;
        }
    }

    FILE *file = fopen(FILE_TRACK, "rb");
    if (file == NULL)
    {
        printf("Error opening file for writing.\n");
        return 0;
    }
    struct Bank_Account account;
    while (fread(&account, sizeof(struct Bank_Account), 1, file))
    {
        if (strcmp(account.pesel, pesel) == 0)
        {
            printf("This pesel already exists.\n");
            fclose(file);
            return 0;
        }
    }
    fclose(file);
    return 1;
}

int count_accounts()
{
    int number_of_accounts = 0;
    FILE *file = fopen(FILE_TRACK, "r");
    if (file == NULL)
    {
        printf("Error opening file for writing.\n");
        return 0;
    }
    struct Bank_Account account;
    fseek(file, 0, SEEK_SET);
    while (fread(&account, sizeof(struct Bank_Account), 1, file))
    {
        number_of_accounts++;
    }
    fclose(file);
    return number_of_accounts;
}

int check_if_account_exist(FILE *file, struct Bank_Account *bank_account, int checked_account_number)
{
    rewind(file);
    while (fread(bank_account, sizeof(struct Bank_Account), 1, file))
    {
        if (bank_account->account_number == checked_account_number)
        {
            return 1;
        }
    }
    return 0;
}

int find_account(FILE *file, struct Bank_Account *bank_account, int operation)
{
    int input_account_number;
    int is_correct_account = 0;

    do
    {
        if (is_correct_account)
        {
            printf("No such account found. ");
        }
        is_correct_account = 1;
        printf("Enter account number:\n");
        if (!scanf("%d", &input_account_number))
        {
            printf("Invalid account. ");
            is_correct_account = 0;
            clear_buffer();
        }
        if (is_correct_account)
        {
            is_correct_account = check_if_account_exist(file, bank_account, input_account_number);
        }
        if (is_correct_account)
        {
            fseek(file, 0, SEEK_SET);
            while (fread(bank_account, sizeof(struct Bank_Account), 1, file))
            {
                if (bank_account->account_number == input_account_number)
                {
                    float current_balance_regular_account = ((float)bank_account->balance_regular_account) / 100;
                    float current_balance_savings_sub_account = ((float)bank_account->balance_savings_sub_account) / 100;
                    if (operation == DEPOSIT_OPERATION)
                    {
                        if (current_balance_regular_account == MAXIMUM_AMOUNT_OF_MONEY)
                        {
                            printf("The account has maximum amount of money.\n");
                            is_correct_account = 0;
                        }
                    }
                    else if (operation == WITHDRAWAL_OPERATION)
                    {
                        if (current_balance_regular_account == 0)
                        {
                            printf("The account has no money.\n");
                            is_correct_account = 0;
                        }
                    }
                    else if (operation == SUB_ACCOUNT_OPERATION)
                    {
                        if (current_balance_regular_account == MAXIMUM_AMOUNT_OF_MONEY || current_balance_savings_sub_account == 0)
                        {
                            can_transfer_from_sub_account = 0;
                        }
                        if (current_balance_savings_sub_account == MAXIMUM_AMOUNT_OF_MONEY || current_balance_regular_account == 0)
                        {
                            can_transfer_to_sub_account = 0;
                        }
                        if (can_transfer_to_sub_account == 0 && can_transfer_from_sub_account == 0)
                        {
                            is_correct_account = 0;
                        }
                    }
                    break;
                }
            }
        }
    } while (!is_correct_account);

    return input_account_number;
}

int check_amount_decimal_places(float amount_of_money)
{
    int decimal_places = 0;
    float temp = amount_of_money;
    while (temp != (int)temp)
    {
        decimal_places++;
        temp *= 10;
    }
    if (decimal_places > 2)
    {
        printf("Amount should have at most 2 digits after the decimal point.\n");
        return 0;
    }
    return 1;
}

float get_money_to_deposit_withdraw(FILE *file, struct Bank_Account *bank_account, int user_input_account_number, int operation)
{
    float amount;
    int is_correct_amount_of_money = 1;

    do
    {
        if (is_correct_amount_of_money == 0)
        {
            printf("Invalid amount of money. ");
        }
        is_correct_amount_of_money = 1;
        printf("Enter the amount of money:\n");
        if (!scanf("%f", &amount) || amount <= 0)
        {
            clear_buffer();
            printf("Invalid input. Please enter a valid amount.\n");
            is_correct_amount_of_money = 0;
        }
        else
        {
            is_correct_amount_of_money = check_amount_decimal_places(amount);
        }
        if (is_correct_amount_of_money)
        {
            fseek(file, 0, SEEK_SET);
            while (fread(bank_account, sizeof(struct Bank_Account), 1, file))
            {
                if (bank_account->account_number == user_input_account_number)
                {
                    float current_balance_regular_account = ((float)bank_account->balance_regular_account) / 100;
                    if (operation == DEPOSIT_OPERATION)
                    {
                        if ((current_balance_regular_account + amount) > MAXIMUM_AMOUNT_OF_MONEY)
                        {
                            printf("The amount of money is unacceptable.\n");
                            is_correct_amount_of_money = 0;
                        }
                    }
                    else if (operation == WITHDRAWAL_OPERATION)
                    {
                        if ((current_balance_regular_account - amount) < 0)
                        {
                            printf("The amount of money is unacceptable.\n");
                            is_correct_amount_of_money = 0;
                        }
                    }
                    break;
                }
            }
        }
    } while (!is_correct_amount_of_money);

    return amount;
}

void create_new_account()
{
    struct Bank_Account account = {0};
    account.account_number = generate_account_number();

    printf("Enter name: ");
    fgets(account.name, sizeof(account.name), stdin);
    while (!is_valid_name_surname(account.name, MAX_LENGTH_OF_NAME_AND_SURNAME))
    {
        printf("Invalid input. Enter name again: ");
        fgets(account.name, sizeof(account.name), stdin);
    }

    printf("Enter surname: ");
    fgets(account.surname, sizeof(account.surname), stdin);
    while (!is_valid_name_surname(account.surname, MAX_LENGTH_OF_NAME_AND_SURNAME))
    {
        printf("Invalid input. Enter surname again: ");
        fgets(account.surname, sizeof(account.surname), stdin);
    }

    printf("Enter address: ");
    fgets(account.address, sizeof(account.address), stdin);
    while (!is_valid_address(account.address, MAX_LENGTH_OF_ADDRESS))
    {
        printf("Invalid input. Enter address again: ");
        fgets(account.address, sizeof(account.address), stdin);
    }

    printf("Enter PESEL: ");
    while (!scanf("%12s", account.pesel) || !is_valid_pesel(account.pesel))
    {
        printf("Invalid input. Enter PESEL again: ");
    }

    account.balance_regular_account = 0.0;
    account.balance_savings_sub_account = 0.0;

    int user_decision = ask_for_confirmation();

    if (user_decision == 1)
    {
        FILE *file = fopen(FILE_TRACK, "ab");
        if (file == NULL)
        {
            printf("Error opening file for writing.\n");
            return;
        }
        fwrite(&account, sizeof(struct Bank_Account), 1, file);
        printf("Account created successfully.\n");
        fclose(file);
    }
}

void list_all_accounts()
{
    FILE *file = fopen(FILE_TRACK, "r");
    if (file == NULL)
    {
        printf("No accounts found.\n");
        return;
    }

    struct Bank_Account account;
    while (fread(&account, sizeof(struct Bank_Account), 1, file))
    {
        printf("Account number: %d\n", account.account_number);
        printf("Name: %s\n", account.name);
        printf("Surname: %s\n", account.surname);
        printf("Address: %s\n", account.address);
        printf("PESEL: %s\n", account.pesel);
        float float_balance_regular_account = ((float)account.balance_regular_account) / 100;
        printf("Regular balance: %.2f\n", float_balance_regular_account);
        float float_balance_savings_sub_account = ((float)account.balance_savings_sub_account) / 100;
        printf("Savings balance: %.2f\n", float_balance_savings_sub_account);
        printf("----------------------------------------------------------\n");
    }

    fclose(file);
}

void search_for_an_account()
{
    int search_option;
    printf("\nSearch for an account using:\n");
    printf("1 - Account number\n");
    printf("2 - Name\n");
    printf("3 - Surname\n");
    printf("4 - Address\n");
    printf("5 - Identification number (PESEL)\n");
    printf("Choose an option: ");
    while (!scanf("%d", &search_option) || search_option < 1 || search_option > 5)
    {
        printf("Invalid choice\nEnter your choice: ");
        clear_buffer();
    }
    clear_buffer();

    FILE *file = fopen(FILE_TRACK, "rb");
    if (file == NULL)
    {
        printf("No accounts found\n");
        return;
    }

    char search_term[MAXIMUM_LINE_LENGTH];
    printf("Enter the search term: ");
    fgets(search_term, sizeof(search_term), stdin);
    search_term[strcspn(search_term, "\n")] = '\0';

    struct Bank_Account account;
    int is_found = 0;

    while (fread(&account, sizeof(struct Bank_Account), 1, file))
    {
        if ((search_option == 1 && atoi(search_term) == account.account_number) ||
            (search_option == 2 && strcmp(search_term, account.name) == 0) ||
            (search_option == 3 && strcmp(search_term, account.surname) == 0) ||
            (search_option == 4 && strcmp(search_term, account.address) == 0) ||
            (search_option == 5 && strcmp(search_term, account.pesel) == 0))
        {
            is_found = 1;

            printf("\nAccount number: %d\n", account.account_number);
            printf("Name: %s\n", account.name);
            printf("Surname: %s\n", account.surname);
            printf("Address: %s\n", account.address);
            printf("PESEL: %s\n", account.pesel);
            float float_balance_regular_account = ((float)account.balance_regular_account) / 100;
            printf("Regular balance: %.2f\n", float_balance_regular_account);
            float float_balance_savings_sub_account = ((float)account.balance_savings_sub_account) / 100;
            printf("Savings balance: %.2f\n", float_balance_savings_sub_account);
            printf("----------------------------------------------------------\n");
        }
    }
    fclose(file);

    if (!is_found)
    {
        printf("No accounts found\n");
    }
}

void make_a_deposit()
{
    int number_of_accounts = count_accounts();
    if (number_of_accounts != 0)
    {
        FILE *file = fopen(FILE_TRACK, "r+");
        struct Bank_Account account;
        if (file == NULL)
        {
            printf("Error opening file.\n");
            return;
        }

        int input_account_number = find_account(file, &account, DEPOSIT_OPERATION);

        float amount = get_money_to_deposit_withdraw(file, &account, input_account_number, DEPOSIT_OPERATION);

        int user_decision = ask_for_confirmation();

        if (user_decision == 1)
        {
            float current_balance_regular_account = ((float)account.balance_regular_account) / 100;
            current_balance_regular_account += amount;
            int converted_balance_regular_account = (int)(current_balance_regular_account * 100);
            account.balance_regular_account = converted_balance_regular_account;
            fseek(file, -sizeof(struct Bank_Account), SEEK_CUR);
            fwrite(&account, sizeof(struct Bank_Account), 1, file);
            printf("Deposit made successfully\n");
        }
        else
        {
            printf("Deposit canceled\n");
        }
        fclose(file);
    }
    else
    {
        printf("There are no accounts in a file\n");
    }
}

void make_a_withdrawal()
{
    int number_of_accounts = count_accounts();
    if (number_of_accounts != 0)
    {
        FILE *file = fopen(FILE_TRACK, "r+");
        struct Bank_Account account;

        if (file == NULL)
        {
            printf("Error opening file.\n");
            return;
        }

        int input_account_number = find_account(file, &account, WITHDRAWAL_OPERATION);

        float amount = get_money_to_deposit_withdraw(file, &account, input_account_number, WITHDRAWAL_OPERATION);

        int user_decision = ask_for_confirmation();

        if (user_decision == 1)
        {
            float current_balance_regular_account = ((float)account.balance_regular_account) / 100;
            current_balance_regular_account -= amount;
            int converted_balance_regular_account = (int)(current_balance_regular_account * 100);
            account.balance_regular_account = converted_balance_regular_account;
            fseek(file, -sizeof(struct Bank_Account), SEEK_CUR);
            fwrite(&account, sizeof(struct Bank_Account), 1, file);
            printf("Withdrawal made successfully\n");
        }
        else
        {
            printf("Withdrawal canceled\n");
        }

        fclose(file);
    }
    else
    {
        printf("There are no accounts in a file\n");
    }
}

void make_a_money_transfer_between_two_accounts()
{
    int number_of_accounts = count_accounts();
    if (number_of_accounts > 1)
    {
        FILE *file = fopen(FILE_TRACK, "r+");
        struct Bank_Account sender_account;
        struct Bank_Account receiver_account;

        if (file == NULL)
        {
            printf("Error opening file.\n");
            return;
        }

        int sender_account_number = find_account(file, &sender_account, WITHDRAWAL_OPERATION);

        int receiver_account_number = find_account(file, &receiver_account, DEPOSIT_OPERATION);

        float amount;
        int is_correct_amount_of_money;

        do
        {
            is_correct_amount_of_money = 1;
            if (is_correct_amount_of_money == 0)
            {
                printf("Invalid amount of money. ");
            }
            printf("Enter the amount of money you want to transfer:\n");
            if (!scanf("%f", &amount) || amount <= 0)
            {
                clear_buffer();
                printf("Invalid input. Please enter a valid amount.\n");
                is_correct_amount_of_money = 0;
            }
            else
            {
                is_correct_amount_of_money = check_amount_decimal_places(amount);
            }
            if (is_correct_amount_of_money)
            {
                fseek(file, 0, SEEK_SET);
                while (fread(&sender_account, sizeof(struct Bank_Account), 1, file))
                {
                    if (sender_account.account_number == sender_account_number)
                    {
                        float current_sender_balance_regular_account = ((float)sender_account.balance_regular_account) / 100;
                        if ((current_sender_balance_regular_account - amount) < 0)
                        {
                            printf("The amount of money you want to transfer is unacceptable.\n");
                            is_correct_amount_of_money = 0;
                        }
                        break;
                    }
                }
                fseek(file, 0, SEEK_SET);
                while (fread(&receiver_account, sizeof(struct Bank_Account), 1, file))
                {
                    if (receiver_account.account_number == receiver_account_number)
                    {
                        float current_receiver_balance_regular_account = ((float)receiver_account.balance_regular_account) / 100;
                        if ((current_receiver_balance_regular_account + amount) > MAXIMUM_AMOUNT_OF_MONEY)
                        {
                            printf("The amount of money you want to transfer is unacceptable.\n");
                            is_correct_amount_of_money = 0;
                        }
                        break;
                    }
                }
            }
        } while (!is_correct_amount_of_money);

        int user_decision = ask_for_confirmation();

        if (user_decision == 1)
        {
            fseek(file, 0, SEEK_SET);
            while (fread(&sender_account, sizeof(struct Bank_Account), 1, file))
            {
                if (sender_account.account_number == sender_account_number)
                {
                    float current_sender_balance_regular_account = ((float)sender_account.balance_regular_account) / 100;
                    current_sender_balance_regular_account -= amount;
                    int converted_sender_balance_regular_account = (int)(current_sender_balance_regular_account * 100);
                    sender_account.balance_regular_account = converted_sender_balance_regular_account;
                    fseek(file, -sizeof(struct Bank_Account), SEEK_CUR);
                    fwrite(&sender_account, sizeof(struct Bank_Account), 1, file);
                    break;
                }
            }

            fseek(file, 0, SEEK_SET);
            while (fread(&receiver_account, sizeof(struct Bank_Account), 1, file))
            {
                if (receiver_account.account_number == receiver_account_number)
                {
                    float current_receiver_balance_regular_account = ((float)receiver_account.balance_regular_account) / 100;
                    current_receiver_balance_regular_account += amount;
                    int converted_receiver_balance_regular_account = (int)(current_receiver_balance_regular_account * 100);
                    receiver_account.balance_regular_account = converted_receiver_balance_regular_account;
                    fseek(file, -sizeof(struct Bank_Account), SEEK_CUR);
                    fwrite(&receiver_account, sizeof(struct Bank_Account), 1, file);
                    break;
                }
            }
            printf("Money transfer made successfully\n");
        }
        else
        {
            printf("Money transfer canceled\n");
        }

        fclose(file);
    }
    else
    {
        printf("There are less than 2 accounts in a file\n");
    }
}

void make_a_money_transfer_to_from_savings_sub_account()
{
    can_transfer_from_sub_account = 1;
    can_transfer_to_sub_account = 1;
    int number_of_accounts = count_accounts();
    if (number_of_accounts != 0)
    {
        FILE *file = fopen(FILE_TRACK, "r+");
        struct Bank_Account account;

        if (file == NULL)
        {
            printf("Error opening file.\n");
            return;
        }

        int input_account_number = find_account(file, &account, SUB_ACCOUNT_OPERATION);

        int user_choice;
        int is_correct_choice = 1;
        do
        {
            if (!is_correct_choice)
            {
                printf("Invalid input.\n");
            }
            is_correct_choice = 0;
            printf("If you want to make a money transfer TO savings sub account: type - 1\n");
            printf("If you want to make a money transfer FROM savings sub account: type - 2\n");
            if (scanf("%d", &user_choice) && (user_choice == 1 || user_choice == 2))
            {
                is_correct_choice = 1;
            }
            clear_buffer();
            if (is_correct_choice)
            {
                if (user_choice == 1 && can_transfer_to_sub_account == 0)
                {
                    is_correct_choice = 0;
                }
                else if (user_choice == 2 && can_transfer_from_sub_account == 0)
                {
                    is_correct_choice = 0;
                }
            }
        } while (!is_correct_choice);

        float amount;
        int is_correct_amount_of_money;

        do
        {
            is_correct_amount_of_money = 1;
            if (is_correct_amount_of_money == 0)
            {
                printf("Invalid amount of money. ");
            }
            printf("Enter the amount of money you want to transfer:\n");
            if (!scanf("%f", &amount) || amount <= 0)
            {
                clear_buffer();
                printf("Invalid input. Please enter a valid amount.\n");
                is_correct_amount_of_money = 0;
            }
            else
            {
                is_correct_amount_of_money = check_amount_decimal_places(amount);
            }
            if (is_correct_amount_of_money)
            {
                fseek(file, 0, SEEK_SET);
                while (fread(&account, sizeof(struct Bank_Account), 1, file))
                {
                    if (account.account_number == input_account_number)
                    {
                        float current_balance_regular_account = ((float)account.balance_regular_account) / 100;
                        float current_balance_savings_sub_account = ((float)account.balance_savings_sub_account) / 100;
                        if (user_choice == 1)
                        {
                            if ((current_balance_regular_account - amount) < 0 || (current_balance_savings_sub_account + amount) > MAXIMUM_AMOUNT_OF_MONEY)
                            {
                                printf("The amount of money you want to transfer is unacceptable.\n");
                                is_correct_amount_of_money = 0;
                            }
                        }
                        else
                        {
                            if ((current_balance_regular_account + amount) > MAXIMUM_AMOUNT_OF_MONEY || (current_balance_savings_sub_account - amount) < 0)
                            {
                                printf("The amount of money you want to transfer is unacceptable.\n");
                                is_correct_amount_of_money = 0;
                            }
                        }
                        break;
                    }
                }
            }
        } while (!is_correct_amount_of_money);

        int user_decision = ask_for_confirmation();

        if (user_decision == 1)
        {
            float current_balance_regular_account = ((float)account.balance_regular_account) / 100;
            float current_balance_savings_sub_account = ((float)account.balance_savings_sub_account) / 100;

            if (user_choice == 1)
            {
                current_balance_regular_account -= amount;
                current_balance_savings_sub_account += amount;
                int converted_balance_regular_account = (int)(current_balance_regular_account * 100);
                int converted_balance_savings_sub_account = (int)(current_balance_savings_sub_account * 100);
                account.balance_regular_account = converted_balance_regular_account;
                account.balance_savings_sub_account = converted_balance_savings_sub_account;
                fseek(file, -sizeof(struct Bank_Account), SEEK_CUR);
                fwrite(&account, sizeof(struct Bank_Account), 1, file);
            }
            else
            {
                current_balance_regular_account += amount;
                current_balance_savings_sub_account -= amount;
                int converted_balance_regular_account = (int)(current_balance_regular_account * 100);
                int converted_balance_savings_sub_account = (int)(current_balance_savings_sub_account * 100);
                account.balance_regular_account = converted_balance_regular_account;
                account.balance_savings_sub_account = converted_balance_savings_sub_account;
                fseek(file, -sizeof(struct Bank_Account), SEEK_CUR);
                fwrite(&account, sizeof(struct Bank_Account), 1, file);
            }
            printf("Money transfer made successfully\n");
        }
        else
        {
            printf("Money transfer canceled\n");
        }
        fclose(file);
    }
    else
    {
        printf("There are no accounts in a file\n");
    }
}

void select_operation()
{
    int scanf_result;
    int operation;
    display_start_screen();
    printf("Enter the operation number: \n");
    while ((scanf_result = scanf("%d", &operation)) == 1 && scanf_result != EOF)
    {
        clear_buffer();
        switch (operation)
        {
        case 1:
            create_new_account();
            break;
        case 2:
            list_all_accounts();
            break;
        case 3:
            search_for_an_account();
            break;
        case 4:
            make_a_deposit();
            break;
        case 5:
            make_a_withdrawal();
            break;
        case 6:
            make_a_money_transfer_between_two_accounts();
            break;
        case 7:
            make_a_money_transfer_to_from_savings_sub_account();
            break;
        case 8:
            printf("\n\nGoodbye. See you next time\n");
            is_exit = 0;
            return;
        default:
            printf("Invalid operation number. Please enter a valid operation number.\n");
            break;
        }
        display_start_screen();
        printf("\nEnter the operation number: ");
    }
    printf("Exiting the program...\n");
}

int main()
{
    FILE *file = fopen(FILE_TRACK, "a");
    if (file == NULL)
    {
        printf("Error opening file for writing.\n");
        return 0;
    }
    fclose(file);
    select_operation();
    return 0;
}
