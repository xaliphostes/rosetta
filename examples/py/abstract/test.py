import rosetta_example as rosetta

def main():
    try:
        d = rosetta.Derived()
        d.run()
    except Exception as e:
        print(f"\n✗ Test failed with error: {e}")
        import traceback
        traceback.print_exc()
        return 1
    
    return 0

if __name__ == "__main__":
    exit(main())