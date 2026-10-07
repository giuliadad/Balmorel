"""Create the extreme-case scenario folders (in the Balmorel root, next to base_dc_4 etc.).

Every scenario starts as a copy of base_dc_4 (data/*.inc, model/Balmorel.gms, model/cplex.op*, investment.sh) with the
simulated time set replaced by the one used for the extreme cases: only the year 2035, the weeks S02 and S28 and three
days per week every 3 hours (Thursday-Saturday, T073, T076, ..., T142, as in base_dc_8).
Then the few input files that define the extreme parameter are overridden. Files that are not in a scenario's data folder
are read from base/data by the model.

    ext_reference                   no change: the run the others are compared with
    ext_no_pv                       no PV (solar heat stays)
    ext_onshore_wind_capex_x10      investment cost of new onshore wind x 10
    ext_no_transmission_expansion   no new transmission capacity
    ext_no_france_interconnectors   no links to France, existing or new

Run from anywhere:   python make_extreme_cases.py      (it overwrites the ext_* folders it creates)
"""
import re
import shutil
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
BASE_DATA = ROOT / "base" / "data"
REFERENCE = ROOT / "base_dc_4"                             # data, model files and job script
SEASONS = ["S02", "S28"]                                   # simulated weeks (one winter, one summer week)
TIME_FILE = ROOT / "base_dc_8" / "data" / "T.inc"          # Thursday-Saturday every 3 hours (T073, T076, ..., T142)
YEARS = [2035]
ENC = "latin1"                      # the .inc files are not UTF-8; latin1 keeps every byte as it is

NUMBER = r"-?\d+(?:\.\d+)?(?:[eE][-+]?\d+)?"
LIST_ROW = re.compile(rf"^(\s*\S+\s+)({NUMBER})(\s*)$")                          # '  AREA_NAME   1234.5'


def read(path):
    return Path(path).read_bytes().decode(ENC)


def write(path, text):
    Path(path).parent.mkdir(parents=True, exist_ok=True)
    Path(path).write_bytes(text.encode(ENC))


def new_scenario(name):
    """Scenario folder = copy of the reference scenario with the time set of the extreme cases. The input files (data/) are
    rebuilt from scratch; model/results (downloaded results of a run) is never touched."""
    assert name.startswith("ext_"), "only folders created by this script are overwritten"
    folder = ROOT / name
    if (folder / "data").exists():
        shutil.rmtree(folder / "data")
    (folder / "data").mkdir(parents=True)
    (folder / "model").mkdir(exist_ok=True)
    for f in (REFERENCE / "data").glob("*.inc"):
        shutil.copyfile(f, folder / "data" / f.name)
    write(folder / "data" / "S.inc",
          "SET S(SSS)  'Seasons in the simulation'\r\n\r\n/\r\n" + ",".join(SEASONS) + "\r\n/;\r\n")
    shutil.copyfile(TIME_FILE, folder / "data" / "T.inc")
    years = ",".join(str(y) for y in YEARS)
    write(folder / "data" / "Y.inc",
          'SET  Y(YYY) "Years in the simulation"\r\n/\r\n' + years + "\r\n\r\n\r\n/;\r\n")
    for f in ["Balmorel.gms", "cplex.op2", "cplex.op4"]:
        shutil.copyfile(REFERENCE / "model" / f, folder / "model" / f)
    job = (REFERENCE / "investment.sh").read_bytes().replace(REFERENCE.name.encode(), name.encode())
    for old, new in [(b'rusage[mem=6GB]', b'rusage[mem=4GB]'), (b'-M 6GB', b'-M 4GB')]:      # memory per core: 4 GB (reference: 6 GB)
        assert job.count(old) == 1, f"{old!r} not found in the job script of {REFERENCE.name}"
        job = job.replace(old, new)
    (folder / "investment.sh").write_bytes(job)
    return folder


def comment(text):
    return "".join(f"* {line}\n" for line in text.strip().splitlines())


# ---------------------------------------------------------------------------------------------------------------------
# 1. No PV: all PV full-load hours are set to 0 (solar heat is not touched)
# ---------------------------------------------------------------------------------------------------------------------
def zero_pv(folder):
    """PV (SOLEFLH) set to 0 full-load hours; solar heat (SOLHFLH) is left as it is.
    The SOLE_VAR_T profile is left untouched on purpose: with a non-zero profile the model still writes the dispatch
    equation (generation = FLH x capacity x profile / sum of profile = 0). With an all-zero profile that equation is
    dropped for the area and new PV capacity would be left without an upper bound on its generation."""
    out, n = [], 0
    for line in read(BASE_DATA / "SOLEFLH.inc").splitlines(keepends=True):
        m = LIST_ROW.match(line.rstrip("\r\n"))
        if m:
            line = m.group(1) + "0" + m.group(3) + line[len(line.rstrip("\r\n")):]
            n += 1
        out.append(line)
    header = comment("EXTREME CASE ext_no_pv: all PV full-load hours set to 0 (no PV anywhere; solar heat is unchanged).\n"
                     f"Copy of base/data/SOLEFLH.inc with every value replaced by 0 ({n} entries).")
    write(folder / "data" / "SOLEFLH.inc", header + "".join(out))
    return n


# ---------------------------------------------------------------------------------------------------------------------
# 2. Onshore wind investment cost x 10
# ---------------------------------------------------------------------------------------------------------------------
def onshore_wind_technologies():
    """New-build onshore wind technologies (rows of GDATA with type GWND and '_ONS_' in the name, not the existing ones)."""
    names = []
    for f in ["GDATA.inc", "GDATA_renewable.inc"]:
        for line in read(BASE_DATA / f).splitlines():
            tokens = line.split()
            if len(tokens) > 2 and tokens[1] == "GWND" and "_ONS_" in tokens[0] and "Existing" not in tokens[0]:
                names.append(tokens[0])
    return sorted(set(names))


def onshore_wind_capex(folder, factor=10):
    """GINVCOST(area,technology) overrides the default GDATA(technology,'GDINVCOST0') where it is non-zero (see
    Balmorelbb4.inc). It is set to factor x GDINVCOST0 for every allowed (area, new onshore wind technology) pair. GDATA
    and AGKN are read before GINVCOST.inc is included, so the factor is applied to whatever the data hold. The base
    GINVCOST.inc has no entries, so nothing is lost by replacing it."""
    techs = onshore_wind_technologies()
    body = (comment(f"EXTREME CASE ext_onshore_wind_capex_x{factor}: investment cost of new onshore wind x {factor}.\n"
                    "Applies to the new-build onshore wind technologies below (GDATA type GWND, '_ONS_' in the name; the existing\n"
                    "'..._Existing_RG*' units have no investment cost) in every area where they can be built (AGKN).")
            + "SET ONSWIND_NEW(GGG) 'New-build onshore wind technologies'\n/\n" + "\n".join(techs) + "\n/;\n\n"
            + f"SCALAR ONSWIND_CAPEX_FACTOR 'Factor on the investment cost of new onshore wind' / {factor} /;\n\n"
            + "GINVCOST(AAA,GGG)$(AGKN(AAA,GGG) AND ONSWIND_NEW(GGG)) = ONSWIND_CAPEX_FACTOR * GDATA(GGG,'GDINVCOST0');\n")
    write(folder / "data" / "GINVCOST.inc", body)
    return len(techs)


# ---------------------------------------------------------------------------------------------------------------------
# 3. No transmission expansion
# ---------------------------------------------------------------------------------------------------------------------
def no_transmission_expansion(folder):
    """XINVCOST without any entry = no investment possible between any pair of regions (see the parameter description in
    base/data/XINVCOST.inc and the IXKN definition in Balmorelbb4.inc). Existing capacities (XKFX) are unchanged."""
    # An empty TABLE statement (like the base GINVCOST.inc): the symbol then counts as assigned for GAMS, so files that
    # only read XINVCOST (e.g. XKRATE.inc) still compile (error 141 otherwise).
    write(folder / "data" / "XINVCOST.inc",
          comment("EXTREME CASE ext_no_transmission_expansion: no new transmission capacity may be built.\n"
                  "XINVCOST has no entries (default: no investment possible). Existing capacity XKFX is unchanged.")
          + "TABLE XINVCOST(YYY,IRRRE,IRRRI)  'Investment cost in new transmission capacity (Money/MW): none'\n"
          + "* (no entries)\n\n")


# ---------------------------------------------------------------------------------------------------------------------
# 4. France without interconnectors
# ---------------------------------------------------------------------------------------------------------------------
def parse_table(lines, name):
    """Position-based reader of a GAMS TABLE(YYY, region, region): returns (header columns, rows).
    rows: list of (line index, label list, [(column name, start, end, text), ...])."""
    start = next(i for i, l in enumerate(lines) if re.match(rf"\s*TABLE\s+{name}\b", l, re.I))
    head = next(i for i in range(start + 1, len(lines)) if lines[i].strip() and not lines[i].lstrip().startswith("*"))
    cols = [(m.group(), m.start(), m.end()) for m in re.finditer(r"\S+", lines[head])]
    first = cols[0][1] - 1
    rows = []
    for i in range(head + 1, len(lines)):
        line = lines[i]
        if line.strip().startswith(";"):
            break
        if not line.strip() or line.lstrip().startswith("*"):
            continue
        labels = line[:first].replace(".", " ").split()
        cells = []
        for m in re.finditer(r"\S+", line[first:]):
            s, e = m.start() + first, m.end() + first
            hit = [c for c in cols if not (e <= c[1] or s >= c[2])]
            assert len(hit) == 1, f"{name}: entry {m.group()!r} in line {i + 1} does not fall under exactly one column"
            cells.append((hit[0][0], s, e, m.group()))
        rows.append((i, labels, cells))
    return cols, rows


def remove_country_links(text, table, country):
    """Blank every entry of the table that has `country` as exporting or importing region (the alignment of the table is kept)."""
    lines = text.split("\n")
    _, rows = parse_table(lines, table)
    removed = []
    for i, labels, cells in rows:
        row_region = labels[-1]
        for col, s, e, value in cells:
            if country in (row_region, col):
                lines[i] = lines[i][:s] + " " * (e - s) + lines[i][e:]
                removed.append((row_region, col, value))
    return "\n".join(lines), removed


def comment_out_country_statements(text, table, country):
    """XKFX.inc and XINVCOST.inc continue after the table with GAMS statements (planned interconnectors added for later
    years, e.g. XKFX(YYY,'FR','UK')$(YVALUE(YYY) GE 2022) = ... + 1400). Statements that name `country` are commented out."""
    lines = text.split("\n")
    start = next(i for i, l in enumerate(lines) if re.match(rf"\s*TABLE\s+{table}\b", l, re.I))
    end = next(i for i in range(start, len(lines)) if lines[i].strip().startswith(";"))      # end of the table
    commented = []
    for i in range(end + 1, len(lines)):
        if lines[i].strip() and not lines[i].lstrip().startswith("*") and f"'{country}'" in lines[i]:
            commented.append(lines[i].strip())
            lines[i] = "*(removed in ext_no_france_interconnectors) " + lines[i]
    return "\n".join(lines), commented


def no_france_interconnectors(folder, country="FR"):
    """All transmission links of France are removed: the entries of the XKFX table and the later XKFX statements that add
    interconnectors (existing capacity), and the corresponding XINVCOST entries (no new capacity)."""
    report = {}
    for table in ["XKFX", "XINVCOST"]:
        text, removed = remove_country_links(read(BASE_DATA / f"{table}.inc"), table, country)
        text, commented = comment_out_country_statements(text, table, country)
        note = comment(f"EXTREME CASE ext_no_france_interconnectors: every {table} entry with {country} as sending or receiving\n"
                       f"region is blanked ({len(removed)} table entries: " + ", ".join(sorted({f'{a}->{b}' for a, b, _ in removed}))
                       + f") and {len(commented)} later statements that name {country} are commented out.")
        write(folder / "data" / f"{table}.inc", note + text)
        report[table] = (sorted(removed), commented)
    return report


# ---------------------------------------------------------------------------------------------------------------------
# Checks on the generated files
# ---------------------------------------------------------------------------------------------------------------------
def table_cells(text, table):
    _, rows = parse_table(text.split("\n"), table)
    return {(labels[0], labels[-1], col): value for _, labels, cells in rows for col, _, _, value in cells}


def check_france(folder, country="FR"):
    for table in ["XKFX", "XINVCOST"]:
        before = table_cells(read(BASE_DATA / f"{table}.inc"), table)
        after = table_cells(read(folder / "data" / f"{table}.inc"), table)
        gone = set(before) - set(after)
        assert set(after) - set(before) == set(), f"{table}: new entries appeared"
        assert all(country in (k[1], k[2]) for k in gone), f"{table}: entries other than {country} links were removed"
        assert not any(country in (k[1], k[2]) for k in after), f"{table}: {country} links remain"
        assert all(after[k] == before[k] for k in after), f"{table}: values changed"
        text = read(folder / "data" / f"{table}.inc").split("\n")
        end = next(i for i, l in enumerate(text) if i > 0 and l.strip().startswith(";") and i > next(
            j for j, t in enumerate(text) if re.match(rf"\s*TABLE\s+{table}\b", t, re.I)))
        left = [l for l in text[end + 1:] if l.strip() and not l.lstrip().startswith("*") and f"'{country}'" in l]
        assert not left, f"{table}: statements naming {country} remain: {left[:2]}"
        print(f"  {table}: {len(before)} table entries -> {len(after)}; removed {len(gone)} {country} links; "
              f"{sum(1 for l in text if l.startswith('*(removed'))} later statements commented out")


def check_pv(folder):
    rows = [m.group(2) for l in read(folder / "data" / "SOLEFLH.inc").splitlines() if (m := LIST_ROW.match(l))]
    assert rows and all(float(v) == 0 for v in rows)
    assert read(folder / "data" / "SOLEFLH.inc") != read(BASE_DATA / "SOLEFLH.inc")
    assert not (folder / "data" / "SOLHFLH.inc").exists(), "solar heat must stay as in base/data"
    print(f"  SOLEFLH.inc: {len(rows)} entries, all 0; solar heat files not overridden")


if __name__ == "__main__":
    import sys

    wanted = set(sys.argv[1:])                                 # e.g.:  python make_extreme_cases.py ext_no_france_interconnectors
    todo = lambda name: not wanted or name in wanted
    print("reference scenario:", REFERENCE.name, "| weeks", SEASONS, "| time set from",
          TIME_FILE.parent.parent.name, "| years", YEARS, "| building:", ", ".join(sorted(wanted)) or "all")

    if todo("ext_reference"):
        folder = new_scenario("ext_reference")
        print(folder.name, "(no change)")

    if todo("ext_no_pv"):
        folder = new_scenario("ext_no_pv")
        print(folder.name, zero_pv(folder), "PV areas set to 0 full-load hours")
        check_pv(folder)

    if todo("ext_onshore_wind_capex_x10"):
        folder = new_scenario("ext_onshore_wind_capex_x10")
        print(folder.name, f"{onshore_wind_capex(folder, 10)} onshore wind technologies get 10 x investment cost")

    if todo("ext_no_transmission_expansion"):
        folder = new_scenario("ext_no_transmission_expansion")
        no_transmission_expansion(folder)
        print(folder.name, "XINVCOST emptied")

    if todo("ext_no_france_interconnectors"):
        folder = new_scenario("ext_no_france_interconnectors")
        report = no_france_interconnectors(folder)
        print(folder.name)
        for table, (removed, commented) in report.items():
            print(f"  {table}: table entries " + ", ".join(f"{a}->{b}" for a, b, _ in removed))
            print(f"  {table}: {len(commented)} statements commented out")
        check_france(folder)
