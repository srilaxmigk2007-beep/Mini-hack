// Bank-account program reads a random-access file sequentially,
// updates data already written to the file, creates new data to
// be placed in the file, and deletes data previously in the file.
// NEW FEATURES:
//   - transferBalance : transfer amount from one account to another
//   - depositWithdraw : deposit or withdraw with amount validation
#include <stdio.h>
#include <stdlib.h>
 
// clientData structure definition
struct clientData
{
    unsigned int acctNum; // account number
    char lastName[15];    // account last name
    char firstName[10];   // account first name
    double balance;       // account balance
};                        // end structure clientData
 
// prototypes
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void transferBalance(FILE *fPtr);   // NEW: transfer balance between two accounts
void depositWithdraw(FILE *fPtr);   // NEW: deposit or withdraw with validation
 
int main(int argc, char *argv[])
{
    FILE *cfPtr;
    unsigned int choice;
 
    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL)
    {
        printf("%s: File could not be opened.\n", argv[0]);
        exit(-1);
    }
 
    while ((choice = enterChoice()) != 5)
    {
        switch (choice)
        {
        case 1:
            textFile(cfPtr);
            break;
        case 2:
            updateRecord(cfPtr);
            break;
        case 3:
            newRecord(cfPtr);
            break;
        case 4:
            deleteRecord(cfPtr);
            break;
        case 6:
            transferBalance(cfPtr);
            break;
        case 7:
            depositWithdraw(cfPtr);
            break;
        default:
            puts("Incorrect choice");
            break;
        }
    }
 
    fclose(cfPtr);
}
 
void textFile(FILE *readPtr)
{
    FILE *writePtr;
    int result;
    struct clientData client = {0, "", "", 0.0};
 
    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
    }
    else
    {
        rewind(readPtr);
        fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
 
        while (!feof(readPtr))
        {
            result = fread(&client, sizeof(struct clientData), 1, readPtr);
            if (result != 0 && client.acctNum != 0)
            {
                fprintf(writePtr, "%-6d%-16s%-11s%10.2f\n",
                        client.acctNum, client.lastName,
                        client.firstName, client.balance);
            }
        }
        fclose(writePtr);
    }
}
 
void updateRecord(FILE *fPtr)
{
    unsigned int account;
    double transaction;
    struct clientData client = {0, "", "", 0.0};
 
    printf("%s", "Enter account to update ( 1 - 100 ): ");
    scanf("%d", &account);
 
    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);
 
    if (client.acctNum == 0)
    {
        printf("Account #%d has no information.\n", account);
    }
    else
    {
        printf("%-6d%-16s%-11s%10.2f\n\n",
               client.acctNum, client.lastName,
               client.firstName, client.balance);
 
        printf("%s", "Enter charge ( + ) or payment ( - ): ");
        scanf("%lf", &transaction);
        client.balance += transaction;
 
        printf("%-6d%-16s%-11s%10.2f\n",
               client.acctNum, client.lastName,
               client.firstName, client.balance);
 
        fseek(fPtr, -sizeof(struct clientData), SEEK_CUR);
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    }
}
 
void deleteRecord(FILE *fPtr)
{
    struct clientData client;
    struct clientData blankClient = {0, "", "", 0};
    unsigned int accountNum;
 
    printf("%s", "Enter account number to delete ( 1 - 100 ): ");
    scanf("%d", &accountNum);
 
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);
 
    if (client.acctNum == 0)
    {
        printf("Account %d does not exist.\n", accountNum);
    }
    else
    {
        fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
    }
}
 
void newRecord(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum;
 
    printf("%s", "Enter new account number ( 1 - 100 ): ");
    scanf("%d", &accountNum);
 
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);
 
    if (client.acctNum != 0)
    {
        printf("Account #%d already contains information.\n", client.acctNum);
    }
    else
    {
        printf("%s", "Enter lastname, firstname, balance\n? ");
        scanf("%14s%9s%lf", client.lastName, client.firstName, &client.balance);
 
        client.acctNum = accountNum;
        fseek(fPtr, (client.acctNum - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    }
}
 
// ============================================================
// NEW FEATURE 1: Transfer Balance Between Two Accounts (Case 6)
//
// Moves a specified amount from a SOURCE account to a
// DESTINATION account with the following validations:
//   - Both accounts must exist
//   - Source and destination must be different accounts
//   - Transfer amount must be greater than zero
//   - Source must have sufficient balance (no overdraft)
//
// Both records are shown before and after the transfer so the
// user can confirm the operation was successful.
// ============================================================
void transferBalance(FILE *fPtr)
{
    struct clientData source      = {0, "", "", 0.0};
    struct clientData destination = {0, "", "", 0.0};
    unsigned int srcAcct, destAcct;
    double transferAmount;
 
    printf("%s", "Enter SOURCE account number ( 1 - 100 ): ");
    scanf("%d", &srcAcct);
 
    printf("%s", "Enter DESTINATION account number ( 1 - 100 ): ");
    scanf("%d", &destAcct);
 
    // validate accounts are different
    if (srcAcct == destAcct)
    {
        printf("Error: Source and destination accounts cannot be the same.\n");
        return;
    }
 
    // validate range
    if (srcAcct < 1 || srcAcct > 100 || destAcct < 1 || destAcct > 100)
    {
        printf("Error: Account numbers must be between 1 and 100.\n");
        return;
    }
 
    // read source account
    fseek(fPtr, (srcAcct - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&source, sizeof(struct clientData), 1, fPtr);
 
    if (source.acctNum == 0)
    {
        printf("Error: Source account #%d does not exist.\n", srcAcct);
        return;
    }
 
    // read destination account
    fseek(fPtr, (destAcct - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&destination, sizeof(struct clientData), 1, fPtr);
 
    if (destination.acctNum == 0)
    {
        printf("Error: Destination account #%d does not exist.\n", destAcct);
        return;
    }
 
    // show balances before transfer
    printf("\n--- Before Transfer ---\n");
    printf("%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("%-6d%-16s%-11s%10.2f\n",
           source.acctNum, source.lastName, source.firstName, source.balance);
    printf("%-6d%-16s%-11s%10.2f\n",
           destination.acctNum, destination.lastName, destination.firstName, destination.balance);
 
    printf("\nEnter amount to transfer: $");
    scanf("%lf", &transferAmount);
 
    // validate amount
    if (transferAmount <= 0)
    {
        printf("Error: Transfer amount must be greater than zero.\n");
        return;
    }
 
    // check sufficient balance
    if (source.balance < transferAmount)
    {
        printf("Error: Insufficient balance in account #%d.\n", srcAcct);
        printf("Available: $%.2f  |  Requested: $%.2f\n", source.balance, transferAmount);
        return;
    }
 
    // perform the transfer
    source.balance      -= transferAmount;
    destination.balance += transferAmount;
 
    // save updated source
    fseek(fPtr, (srcAcct - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&source, sizeof(struct clientData), 1, fPtr);
 
    // save updated destination
    fseek(fPtr, (destAcct - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&destination, sizeof(struct clientData), 1, fPtr);
 
    // show balances after transfer
    printf("\n--- After Transfer ---\n");
    printf("%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("%-6d%-16s%-11s%10.2f\n",
           source.acctNum, source.lastName, source.firstName, source.balance);
    printf("%-6d%-16s%-11s%10.2f\n",
           destination.acctNum, destination.lastName, destination.firstName, destination.balance);
 
    printf("\nTransfer of $%.2f from Account #%d to Account #%d successful!\n",
           transferAmount, srcAcct, destAcct);
}
 
// ============================================================
// NEW FEATURE 2: Deposit & Withdraw with Validation (Case 7)
//
// Provides a safe, dedicated transaction menu for an account.
// User picks D (Deposit) or W (Withdraw) and enters an amount.
//
// Validations performed:
//   - Account must exist
//   - Type must be D or W only
//   - Amount must be greater than zero
//   - Withdrawal is blocked if it exceeds available balance
//     (overdraft protection — balance can never go negative)
//
// Current and updated balance are displayed after each action.
// ============================================================
void depositWithdraw(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum;
    double amount;
    char type;
 
    printf("%s", "Enter account number ( 1 - 100 ): ");
    scanf("%d", &accountNum);
 
    // validate range
    if (accountNum < 1 || accountNum > 100)
    {
        printf("Error: Account number must be between 1 and 100.\n");
        return;
    }
 
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);
 
    // validate account exists
    if (client.acctNum == 0)
    {
        printf("Error: Account #%d does not exist.\n", accountNum);
        return;
    }
 
    // show current balance
    printf("\n--- Current Account Info ---\n");
    printf("%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("%-6d%-16s%-11s%10.2f\n",
           client.acctNum, client.lastName, client.firstName, client.balance);
 
    // get transaction type
    printf("\nTransaction type:\n");
    printf("  D - Deposit\n");
    printf("  W - Withdraw\n");
    printf("Choice: ");
    scanf(" %c", &type);
 
    // validate type
    if (type != 'D' && type != 'd' && type != 'W' && type != 'w')
    {
        printf("Error: Invalid type. Enter D or W only.\n");
        return;
    }
 
    printf("Enter amount: $");
    scanf("%lf", &amount);
 
    // validate amount
    if (amount <= 0)
    {
        printf("Error: Amount must be greater than zero.\n");
        return;
    }
 
    // process deposit
    if (type == 'D' || type == 'd')
    {
        client.balance += amount;
        printf("\nDeposit of $%.2f successful!\n", amount);
    }
    else // process withdrawal with overdraft protection
    {
        if (amount > client.balance)
        {
            printf("\nError: Insufficient funds.\n");
            printf("Requested: $%.2f  |  Available: $%.2f\n", amount, client.balance);
            return;
        }
        client.balance -= amount;
        printf("\nWithdrawal of $%.2f successful!\n", amount);
    }
 
    // write updated record back to file
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);
 
    // show updated balance
    printf("\n--- Updated Account Info ---\n");
    printf("%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("%-6d%-16s%-11s%10.2f\n",
           client.acctNum, client.lastName, client.firstName, client.balance);
}
 
// enable user to input menu choice
unsigned int enterChoice(void)
{
    unsigned int menuChoice;
    printf("%s", "\nEnter your choice\n"
                 "1 - store a formatted text file of accounts called\n"
                 "    \"accounts.txt\" for printing\n"
                 "2 - update an account\n"
                 "3 - add a new account\n"
                 "4 - delete an account\n"
                 "5 - end program\n"
                 "6 - transfer balance between two accounts\n"
                 "7 - deposit or withdraw with validation\n"
                 "? ");
 
    scanf("%u", &menuChoice);
    return menuChoice;
}
