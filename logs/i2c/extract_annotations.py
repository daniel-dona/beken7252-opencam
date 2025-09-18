import re
import argparse
import sys

def parse_i2c_log(file_path):
    """
    Parse I2C log file and extract byte operations by transaction.
    
    Args:
        file_path (str): Path to the .txt log file
        
    Returns:
        list: List of dictionaries, each representing one I2C transaction
    """
    
    # Read all lines from the file
    try:
        with open(file_path, 'r') as f:
            lines = [line.strip() for line in f.readlines() if line.strip()]
    except FileNotFoundError:
        print(f"Error: File '{file_path}' not found.")
        sys.exit(1)
    except Exception as e:
        print(f"Error reading file: {e}")
        sys.exit(1)
    
    transactions = []
    current_transaction = None
    
    # Regular expressions to match different operation types
    data_write_pattern = re.compile(r'Data write:\s*([0-9A-Fa-f]+)')
    data_read_pattern = re.compile(r'Data read:\s*([0-9A-Fa-f]+)')
    
    # Process each line
    for line in lines:
        # Check if this is a new transaction (Start or Stop)
        if 'Start' in line and 'repeat' not in line:
            # Start of a new transaction
            if current_transaction is not None:
                transactions.append(current_transaction)
            
            current_transaction = {
                'bytes_written': [],
                'bytes_read': []
            }
        
        elif 'Stop' in line and current_transaction is not None:
            # End of transaction (Stop)
            transactions.append(current_transaction)
            current_transaction = None
            
        elif 'Data write:' in line and current_transaction is not None:
            # Extract data byte for writing
            data_match = data_write_pattern.search(line)
            if data_match:
                hex_value = data_match.group(1)
                decimal_value = int(hex_value, 16)
                current_transaction['bytes_written'].append(decimal_value)
        
        elif 'Data read:' in line and current_transaction is not None:
            # Extract data byte for reading
            data_match = data_read_pattern.search(line)
            if data_match:
                hex_value = data_match.group(1)
                decimal_value = int(hex_value, 16)
                current_transaction['bytes_read'].append(decimal_value)
    
    # Handle case where file ends without Stop (last transaction)
    if current_transaction is not None:
        transactions.append(current_transaction)
    
    return transactions

def print_transactions(transactions):
    """
    Print formatted output of parsed transactions with only bytes.
    
    Args:
        transactions (list): List of transaction dictionaries
    """
    for i, transaction in enumerate(transactions):
        print(f"Transaction {i + 1}: ", end="")
        
        # Only include bytes_written if they exist
        if transaction['bytes_written']:
            print(f"write: {[hex(b) for b in transaction['bytes_written']]}", end="")
            
        # Only include bytes_read if they exist and we have written data too
        if transaction['bytes_read']:
            if transaction['bytes_written']:
                print(", ", end="")
            print(f"read: {[hex(b) for b in transaction['bytes_read']]}")
        elif not transaction['bytes_written']:
            print("No data")
        else:
            print()

def save_to_file(transactions, output_path):
    """
    Save transactions to a text file with only bytes.
    
    Args:
        transactions (list): List of transaction dictionaries
        output_path (str): Path to output file
    """
    try:
        with open(output_path, 'w') as f:
            for i, transaction in enumerate(transactions):
                f.write(f"Transaction {i + 1}: ")
                
                # Only include bytes_written if they exist
                if transaction['bytes_written']:
                    f.write(f"write: {[hex(b) for b in transaction['bytes_written']]}")
                    
                # Only include bytes_read if they exist and we have written data too
                if transaction['bytes_read']:
                    if transaction['bytes_written']:
                        f.write(", ")
                    f.write(f"read: {[hex(b) for b in transaction['bytes_read']]}")
                elif not transaction['bytes_written']:
                    f.write("No data")
                else:
                    f.write("\n")
                
                f.write("\n")
        print(f"Transactions saved to '{output_path}'")
    except Exception as e:
        print(f"Error saving file: {e}")
        sys.exit(1)

def get_bytes_only(transactions):
    """
    Extract only the write byte arrays for each transaction.
    
    Returns:
        list: List of lists containing bytes written from each transaction
    """
    return [transaction['bytes_written'] for transaction in transactions]

def get_read_bytes_only(transactions):
    """
    Extract only the read byte arrays for each transaction.
    
    Returns:
        list: List of lists containing read bytes from each transaction
    """
    return [transaction['bytes_read'] for transaction in transactions]

def main():
    parser = argparse.ArgumentParser(description='Parse I2C log file and extract byte operations')
    parser.add_argument('input_file', help='Input .txt log file path')
    parser.add_argument('-o', '--output', help='Output file path (optional)')
    parser.add_argument('--bytes-only', action='store_true', help='Output only write byte arrays')
    parser.add_argument('--read-bytes-only', action='store_true', help='Output only read byte arrays')
    
    args = parser.parse_args()
    
    # Parse the I2C log
    transactions = parse_i2c_log(args.input_file)
    
    if args.bytes_only:
        # Output only write byte arrays
        bytes_list = get_bytes_only(transactions)
        for i, bytes_array in enumerate(bytes_list):
            print(f"Transaction {i + 1}: {[hex(b) for b in bytes_array]}")
    elif args.read_bytes_only:
        # Output only read byte arrays
        read_bytes_list = get_read_bytes_only(transactions)
        for i, bytes_array in enumerate(read_bytes_list):
            print(f"Transaction {i + 1}: {[hex(b) for b in bytes_array]}")
    else:
        # Output formatted transactions with only bytes
        print("Parsed I2C Transactions:")
        print("=" * 50)
        print_transactions(transactions)
        
        if args.output:
            save_to_file(transactions, args.output)
        else:
            # If no output file specified, just print to console
            pass

if __name__ == "__main__":
    main()
