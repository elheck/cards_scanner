#!/bin/bash

# =============================================================================
# run_quality_tests.sh - Comprehensive quality check script
# =============================================================================
# Runs all quality checks including:
# - Build warnings check
# - clang-format (code formatting)
# - clang-tidy (static analysis)
# - cppcheck (additional static analysis)
# - Header guards check
# - Unit tests (via ctest)
# =============================================================================

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# Counters
TOTAL_CHECKS=0
PASSED_CHECKS=0
FAILED_CHECKS=0

# Step tracking
CURRENT_STEP=0
MAX_STEPS=6

# Build directory
BUILD_DIR="build"

# Source directories to check
SRC_DIRS="src tests"

# =============================================================================
# Helper functions
# =============================================================================

print_step() {
    CURRENT_STEP=$((CURRENT_STEP + 1))
    echo ""
    echo -e "${CYAN}┌─────────────────────────────────────────────────────────────────────────────┐${NC}"
    echo -e "${CYAN}│${NC} ${BOLD}Step $CURRENT_STEP/$MAX_STEPS:${NC} $1"
    echo -e "${CYAN}└─────────────────────────────────────────────────────────────────────────────┘${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_info() {
    echo -e "  ${BLUE}→${NC} $1"
}

check_result() {
    TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
    if [ $1 -eq 0 ]; then
        PASSED_CHECKS=$((PASSED_CHECKS + 1))
        print_success "$2"
        return 0
    else
        FAILED_CHECKS=$((FAILED_CHECKS + 1))
        print_error "$2"
        return 1
    fi
}

# =============================================================================
# Ensure build directory exists with compile_commands.json
# =============================================================================

ensure_build() {
    if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
        print_info "Building project first (needed for compile_commands.json)..."
        ./rebuild.sh || {
            print_error "Build failed - cannot proceed with quality checks"
            exit 1
        }
    fi
}

# =============================================================================
# Step 1: Build warnings
# =============================================================================

check_build_warnings() {
    print_step "Checking build (compiler warnings)"
    
    print_info "Running rebuild.sh..."
    local build_output
    build_output=$(./rebuild.sh 2>&1)
    local build_result=$?
    
    if [ $build_result -ne 0 ]; then
        echo "$build_output"
        check_result 1 "Build: Failed to compile"
        return 1
    fi
    
    local warning_count=$(echo "$build_output" | grep -c "warning:" || true)
    
    if [ "$warning_count" -eq 0 ]; then
        print_info "Build completed successfully"
        check_result 0 "Build: No compiler warnings"
    else
        echo "$build_output" | grep "warning:" | head -10
        check_result 1 "Build: $warning_count compiler warnings found"
    fi
}

# =============================================================================
# Step 2: clang-format
# =============================================================================

run_clang_format() {
    print_step "Code formatting (clang-format)"
    
    local format_errors=0
    local files_checked=0
    local bad_files=""
    
    print_info "Checking formatting of source files..."
    
    while IFS= read -r -d '' file; do
        files_checked=$((files_checked + 1))
        if ! clang-format --dry-run --Werror "$file" 2>/dev/null; then
            bad_files="$bad_files\n    - $file"
            format_errors=$((format_errors + 1))
        fi
    done < <(find $SRC_DIRS \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \) -print0 2>/dev/null)
    
    print_info "Checked $files_checked files"
    
    if [ $format_errors -eq 0 ]; then
        check_result 0 "clang-format: All $files_checked files properly formatted"
    else
        echo -e "  Files needing formatting:$bad_files"
        print_warning "Run './run_quality_tests.sh --fix-format' to auto-fix"
        check_result 1 "clang-format: $format_errors files need formatting"
    fi
}

# =============================================================================
# Step 3: clang-tidy
# =============================================================================

run_clang_tidy() {
    print_step "Static analysis (clang-tidy)"
    
    ensure_build
    
    local tidy_errors=0
    local files_checked=0
    local tidy_output
    
    print_info "Analyzing source files with clang-tidy..."
    
    while IFS= read -r -d '' file; do
        files_checked=$((files_checked + 1))
        local filename=$(basename "$file")
        echo -n "  [$files_checked] $filename ... "
        
        tidy_output=$(clang-tidy -p="$BUILD_DIR" "$file" 2>&1)
        
        if echo "$tidy_output" | grep -q "error:"; then
            echo -e "${RED}ISSUES${NC}"
            tidy_errors=$((tidy_errors + 1))
            # Show the errors
            echo "$tidy_output" | grep -E "error:" | head -3 | sed 's/^/      /'
        else
            echo -e "${GREEN}OK${NC}"
        fi
    done < <(find src \( -name '*.cpp' \) -print0 2>/dev/null)
    
    echo ""
    if [ $tidy_errors -eq 0 ]; then
        check_result 0 "clang-tidy: All $files_checked files pass static analysis"
    else
        print_warning "Run 'clang-tidy -p=build <file>' for full details"
        check_result 1 "clang-tidy: $tidy_errors/$files_checked files have issues"
    fi
}

# =============================================================================
# Step 4: cppcheck
# =============================================================================

run_cppcheck() {
    print_step "Additional static analysis (cppcheck)"
    
    if ! command -v cppcheck &> /dev/null; then
        print_warning "cppcheck not found - skipping"
        return 0
    fi
    
    print_info "Running cppcheck on source files..."
    
    local cppcheck_output
    cppcheck_output=$(cppcheck \
        --enable=warning,style,performance,portability \
        --suppress=missingIncludeSystem \
        --suppress=unmatchedSuppression \
        --error-exitcode=1 \
        --inline-suppr \
        -I src/detection/include \
        -I src/misc/include \
        -I src/api/include \
        -I src/workflow/include \
        src/ 2>&1)
    local cppcheck_result=$?
    
    if [ $cppcheck_result -ne 0 ]; then
        echo "$cppcheck_output" | grep -v "^Checking" | head -20
        check_result 1 "cppcheck: Issues found"
        return 1
    fi
    
    check_result 0 "cppcheck: No issues found"
}

# =============================================================================
# Step 5: Header guards
# =============================================================================

check_header_guards() {
    print_step "Header guards check (#pragma once)"
    
    local missing_guards=0
    local files_checked=0
    
    print_info "Checking header files for #pragma once..."
    
    while IFS= read -r -d '' file; do
        files_checked=$((files_checked + 1))
        if ! head -5 "$file" | grep -q "#pragma once"; then
            print_error "Missing #pragma once: $file"
            missing_guards=$((missing_guards + 1))
        fi
    done < <(find $SRC_DIRS -name '*.hpp' -print0 2>/dev/null)
    
    print_info "Checked $files_checked header files"
    
    if [ $missing_guards -eq 0 ]; then
        check_result 0 "Header guards: All headers have #pragma once"
    else
        check_result 1 "Header guards: $missing_guards headers missing #pragma once"
    fi
}

# =============================================================================
# Step 6: Unit tests
# =============================================================================

run_unit_tests() {
    print_step "Unit tests (ctest)"
    
    ensure_build
    
    print_info "Running all unit tests..."
    
    local test_output
    test_output=$(ctest --test-dir "$BUILD_DIR" --output-on-failure 2>&1)
    local test_result=$?
    
    # Show test progress
    echo "$test_output" | grep -E "^\s*[0-9]+/[0-9]+ Test" | tail -10
    
    # Extract test summary
    local summary=$(echo "$test_output" | grep -E "tests passed|tests failed" | tail -1)
    echo ""
    print_info "$summary"
    
    if [ $test_result -eq 0 ]; then
        check_result 0 "Unit tests: All tests passed"
    else
        echo ""
        echo "$test_output" | grep -A5 "The following tests FAILED"
        check_result 1 "Unit tests: Some tests failed"
    fi
}

# =============================================================================
# Main execution
# =============================================================================

main() {
    echo -e "${BLUE}"
    echo "╔═══════════════════════════════════════════════════════════════════════════╗"
    echo "║                    MTG Card Scanner - Quality Checks                      ║"
    echo "╚═══════════════════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
    
    # Parse arguments
    RUN_ALL=true
    RUN_FORMAT=false
    RUN_TIDY=false
    RUN_CPPCHECK=false
    RUN_TESTS=false
    RUN_BUILD=false
    RUN_HEADERS=false
    FIX_FORMAT=false
    VERBOSE=false
    
    for arg in "$@"; do
        case $arg in
            --format)
                RUN_ALL=false
                RUN_FORMAT=true
                ;;
            --tidy)
                RUN_ALL=false
                RUN_TIDY=true
                ;;
            --cppcheck)
                RUN_ALL=false
                RUN_CPPCHECK=true
                ;;
            --tests)
                RUN_ALL=false
                RUN_TESTS=true
                ;;
            --build)
                RUN_ALL=false
                RUN_BUILD=true
                ;;
            --headers)
                RUN_ALL=false
                RUN_HEADERS=true
                ;;
            --fix-format)
                FIX_FORMAT=true
                ;;
            --verbose|-v)
                VERBOSE=true
                ;;
            --help|-h)
                echo "Usage: $0 [OPTIONS]"
                echo ""
                echo "Options:"
                echo "  --format      Run clang-format check only"
                echo "  --tidy        Run clang-tidy check only"
                echo "  --cppcheck    Run cppcheck only"
                echo "  --tests       Run unit tests only"
                echo "  --build       Check build warnings only"
                echo "  --headers     Check header guards only"
                echo "  --fix-format  Auto-fix formatting issues"
                echo "  --verbose, -v Show more details"
                echo "  --help, -h    Show this help message"
                echo ""
                echo "Without options, all checks are run."
                exit 0
                ;;
            *)
                print_error "Unknown option: $arg"
                exit 1
                ;;
        esac
    done
    
    # Update MAX_STEPS based on what we're running
    if [ "$RUN_ALL" = false ]; then
        MAX_STEPS=1
    fi
    
    # Auto-fix formatting if requested
    if [ "$FIX_FORMAT" = true ]; then
        MAX_STEPS=1
        print_step "Auto-fixing code formatting"
        print_info "Formatting all source files..."
        find $SRC_DIRS \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \) -exec clang-format -i {} \;
        print_success "Formatting fixed for all files"
        exit 0
    fi
    
    # Run selected checks - fail immediately if any check fails
    if [ "$RUN_ALL" = true ] || [ "$RUN_BUILD" = true ]; then
        check_build_warnings || exit 1
    fi
    
    if [ "$RUN_ALL" = true ] || [ "$RUN_FORMAT" = true ]; then
        run_clang_format || exit 1
    fi
    
    if [ "$RUN_ALL" = true ] || [ "$RUN_TIDY" = true ]; then
        run_clang_tidy || exit 1
    fi
    
    if [ "$RUN_ALL" = true ] || [ "$RUN_CPPCHECK" = true ]; then
        run_cppcheck || exit 1
    fi
    
    if [ "$RUN_ALL" = true ] || [ "$RUN_HEADERS" = true ]; then
        check_header_guards || exit 1
    fi
    
    if [ "$RUN_ALL" = true ] || [ "$RUN_TESTS" = true ]; then
        run_unit_tests || exit 1
    fi
    
    # Print summary
    echo ""
    echo -e "${CYAN}┌─────────────────────────────────────────────────────────────────────────────┐${NC}"
    echo -e "${CYAN}│${NC} ${BOLD}Summary${NC}"
    echo -e "${CYAN}└─────────────────────────────────────────────────────────────────────────────┘${NC}"
    echo ""
    echo -e "  Total checks:  ${BOLD}$TOTAL_CHECKS${NC}"
    echo -e "  Passed:        ${GREEN}$PASSED_CHECKS${NC}"
    echo -e "  Failed:        ${RED}$FAILED_CHECKS${NC}"
    echo ""
    
    if [ $FAILED_CHECKS -eq 0 ]; then
        echo -e "${GREEN}╔═══════════════════════════════════════════════════════════════════════════╗${NC}"
        echo -e "${GREEN}║                         All quality checks passed!                        ║${NC}"
        echo -e "${GREEN}╚═══════════════════════════════════════════════════════════════════════════╝${NC}"
        exit 0
    else
        echo -e "${RED}╔═══════════════════════════════════════════════════════════════════════════╗${NC}"
        echo -e "${RED}║                      Some quality checks failed!                          ║${NC}"
        echo -e "${RED}╚═══════════════════════════════════════════════════════════════════════════╝${NC}"
        exit 1
    fi
}

main "$@"
