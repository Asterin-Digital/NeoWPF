# Security

* Pre-release status.

- NeoWPF does not currently provide comprehensive defense against malicious input.
- Some components reject specific malformed or invalid values. These checks are limited and are not a security guarantee.
- Do not treat NeoWPF as a sandbox or security boundary for untrusted input.
- Validate and limit untrusted data before passing it to NeoWPF.