import subprocess
import sys
from pathlib import Path

def run_cargo_build():
    """Compileaza si ruleaza cargo pentru a verifica daca exista erori."""
    print("========== Compiling Rust project ==========")
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


def run_cargo_test():
    """Ruleaza testele unitare (pentru Task 1 si Task 2)"""
    print("========== Running Rust unit tests ==========")
    result = subprocess.run(
        ["cargo", "test"], 
        stdout=subprocess.PIPE, 
        stderr=subprocess.PIPE,
        text=True
    )
    # Afiseaza warning-uri si erori
    print(result.stdout)
    print(result.stderr)
    if result.returncode != 0:
        print("Eroare la rularea testelor unitare! Oprire program.")
        sys.exit(1)
    return result.stdout

def run_task2_test(db_file, test_idx):
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
    

def test_task2():
    print("========== TASK 3 ==========")
    score = 0
    db_files = [Path("../tests/db/small.db"), Path("../tests/db/big.db")]
    points_per_test = 3.5
    for db_file in db_files:
        print(f"\n--- Testare pentru {db_file.name} ---")
        for i in range(1, 11):
            if run_task2_test(db_file, i):
                score += points_per_test
    print()
    return score
    
def test_task1(stdout_result):
    """stdout_result -> rezultatul testelor unitare, rulate cu `cargo test`"""
    print("========== TASK 1 ==========")
    score = 0

    print("Verificare task 1.1 [citire_secretariat]: ", end='')
    if stdout_result.find("test_citire_secretariat ... ok") >= 0:
        score += 7
        print("OK")
    else:
        print("FAIL")

    print("Verificare task 1.2 [adauga_student]: ", end='')
    if stdout_result.find("test_adauga_student ... ok") >= 0:
        score += 3
        print("OK")
    else:
        print("FAIL")

    print()
    return score

def test_task3(stdout_result):
    """stdout_result -> rezultatul testelor unitare, rulate cu `cargo test`"""
    print("========== TASK 1 ==========")
    score = 0

    for i in range(1, 6):
        print(f"Test with `test{i}`.db: ", end='')
        if stdout_result.find(f"test_cripteaza_studenti::case_{i} ... ok") >= 0:
            score += 4
            print("OK")
        else:
            print("FAIL")
    return score
    


def main():
    run_cargo_build()
    unit_tests_results = run_cargo_test()
    total_score = 0
    total_score += test_task1(unit_tests_results)
    total_score += test_task2()
    total_score += test_task3(unit_tests_results)
    print(f"\nScor total: {total_score}/100")

if __name__ == '__main__':
    main()
