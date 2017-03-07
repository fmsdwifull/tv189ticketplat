#include <stdio.h>  // for printf
#include <SQLAPI.h> // main SQLAPI++ header

int main(int argc, char* argv[])
{
    SAConnection con; // connection object
    SACommand cmd;    // create command object
    
    try
    {
        // connect to database (Oracle in our example)
        con.Connect("192.168.19.223@test", "root", "123456", SA_MySQL_Client);
        // associate a command with connection
        // connection can also be specified in SACommand constructor
        cmd.setConnection(&con);
/*
        // create table
        cmd.setCommandText(
            "Create table test_tbl(fid integer, fvarchar20 varchar(20), fblob blob)");
        cmd.Execute();

        // insert value
        cmd.setCommandText(
            "Insert into test_tbl(fid, fvarchar20) values (1, 'Some string (1)')");
        cmd.Execute();

        // commit changes on success
        con.Commit();

        printf("Table created, row inserted!\n");
        */
        // insert value
        cmd.setCommandText( "Insert into Staff(ID, Name) values (2, 'def')");
        cmd.Execute();

        // commit changes on success
        con.Commit();
    }
    catch(SAException &x)
    {
        // SAConnection::Rollback()
        // can also throw an exception
        // (if a network error for example),
        // we will be ready
        try
        {
            // on error rollback changes
            con.Rollback();
        }
        catch(SAException &)
        {
        }
        // print error message
        printf("%s\n", (const char*)x.ErrText());
    }
    
    return 0;
}
