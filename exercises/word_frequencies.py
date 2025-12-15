def word_counts(sentences: list[str]) -> dict[str, int]:
    """
    Returns a dictionary with words from the input sentences as keys and
    their frequency as values
    """
    result: dict[str, int] = {}
    for sentence in sentences:
        for word in sentence.split(' '):
            result.setdefault(word, 0)
            result[word] += 1
    return result


def compare(result: dict[str, int], gold_std: dict[str, int]):
    err = ""
    if len(result) != len(gold_std):
        err += f"result length ({len(result)}) != gold_std length ({len(gold_std)})"
    added_header = False
    for key in result:
        if not key in gold_std:
            if not added_header:
                if err != "":
                    err += "\n"
                err += "Superfluous words: "
                added_header = True
            else:
                err += ": "
            err += f"{key}"
    added_header = False
    for key in gold_std:
        if not key in result:
            if not added_header:
                if err != "":
                    err += "\n"
                err += "Missing words: "
                added_header = True
            else:
                err += ": "
            err += f"{key}"
    if err != "":
        raise Exception(err)


num_test_case = 1
for (test_case, gold_std, expected_exception) in [
    [
        ["this is my first sentence and my first test",
            "here is another sentence let us see if this is my third thing"],
        {
            "thing": 1, "this": 2, "is": 3, "third": 1, "let": 1,
            "my": 3, "another": 1, "see": 1,  "sentence": 2, "and": 1,
            "first": 2, "test": 1, "here": 1, "if": 1, "us": 1
        },
        None
    ],
    [
        # Should show that "and" is superfluous
        ["this is my first sentence and my first test",
            "here is another sentence let us see if this is my third thing"],
        {
            "thing": 1, "this": 2, "is": 3, "third": 1, "let": 1,
            "my": 3, "another": 1, "see": 1, "sentence": 2,
            "first": 2, "test": 1, "here": 1, "if": 1, "us": 1
        },
        Exception(
            "result length (15) != gold_std length (14)\nSuperfluous words: and")
    ],
    [
        # Should show that "monkey" is missing
        ["this is my first sentence and my first test",
            "here is another sentence let us see if this is my third thing"],
        {
            "thing": 1, "this": 2, "is": 3, "third": 1, "let": 1,
            "my": 3, "another": 1, "see": 1, "sentence": 2, "and": 1,
            "first": 2, "test": 1, "here": 1, "if": 1, "us": 1,
            "monkey": 1
        },
        Exception(
            "result length (15) != gold_std length (16)\nMissing words: monkey")
    ]
]:
    frequencies = word_counts(test_case)
    try:
        compare(frequencies, gold_std)
        print(f"test case {num_test_case}: OK")
    except Exception as e:
        if expected_exception is None or str(e) != str(expected_exception):
            print(
                f"test case {num_test_case}: FAIL:\n\t{str(e).replace('\n', '\n\t')}\n\tfrequencies: {frequencies}\n\tgold_std: {gold_std}")
        else:
            print(f"test case {num_test_case}: OK (got expected exception)")
    num_test_case += 1
