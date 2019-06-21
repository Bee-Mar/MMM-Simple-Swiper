module.exports = {
  env: {
    browser: true,
    es6: true,
    node: true
  },
  extends: "standard",
  globals: {
    Atomics: "readonly",
    SharedArrayBuffer: "readonly"
  },
  parserOptions: {
    ecmaVersion: 2018,
    sourceType: "module"
  },
  rules: {
    quotes: [2, "double"],
    semi: [2, "always"],
    "space-before-function-paren": ["error", "never"]
  }
};
