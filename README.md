[![codecov](https://codecov.io/github/soblin/cpplox/graph/badge.svg?token=KE0TEXDAUH)](https://codecov.io/github/soblin/cpplox)

# cpplox

[codecov](https://app.codecov.io/github/soblin/cpplox/tree/main/src)

## Docker

Environement variables are overrided in the ascending order as follows:

1. command line specified key-values: `docker run -e`, `docker compose -e`
2. `environment:` field(and the values can be referenced from `.env` or `env_file:` files)
3. `ENV` key-values in `Dockerfile`

Secret *value* needs to be specified in `.env` file, which is not git-ignored.

### `.env` file

`.env` and `.env.dev` are loaded in `docker-compose.yml`, and `.env`
