import subprocess
import sys
from pathlib import Path

def run_cargo():
    """Compileaza si ruleaza cargo pentru a verifica daca exista erori."""
    print("Compiling Rust project...")
    result = subprocess.run(
        ["cargo", "build"], 
        stdout=subprocess.PIPE, 
        stderr=subprocess.PIPE,
        text=True
    )
    # Afiseaza warning-uri si erori
    print(result.stdout)
    print(result.stderr)
    if result.returncode != 0:
        print("Eroare la compilare! Oprire program.")
        sys.exit(1)

def run_test(db_file, test_idx):
    """Ruleaza un singur test si returneaza True daca trece, False altfel."""
    input_file = Path(f"../tests/input/test{test_idx}.in")
    output_dir = Path("../tests/output-rust-method")
    output_dir.mkdir(parents=True, exist_ok=True)
    output_file = output_dir / f"{db_file.stem}.test{test_idx}.out"
    ref_file = Path(f"../tests/ref/{db_file.stem}.test{test_idx}.ref")

    # Ruleaza aplicatia Rust
    with input_file.open("r") as fin, output_file.open("w") as fout:
        result = subprocess.run(
            ["cargo", "run", "--", str(db_file)],
            stdin=fin,
            stdout=fout,
            stderr=subprocess.PIPE,
            text=True
        )
    if result.returncode != 0:
        print(f"Eroare la rularea testului {test_idx} cu {db_file.name}")
        print(result.stderr)
        return False

    # Compara output cu fisierul de referinta
    diff_result = subprocess.run(
        ["diff", str(output_file), str(ref_file)],
        stdout=subprocess.PIPE,
        text=True
    )
    if diff_result.returncode == 0:
        print(f"Test {test_idx} cu {db_file.name}: OK")
        return True
    else:
        print(f"Test {test_idx} cu {db_file.name}: FAILED")
        return False

def main():
    run_cargo()
    db_files = [Path("../tests/db/small.db"), Path("../tests/db/big.db")]
    total_score = 0
    points_per_test = 3.5
    for db_file in db_files:
        print(f"\n--- Testare pentru {db_file.name} ---")
        for i in range(1, 11):
            if run_test(db_file, i):
                total_score += points_per_test

    print(f"\nScor total: {total_score}/70")

if __name__ == '__main__':
    main()
