# Hyper C Markup eXtension

The goal of this project is to implement a minimalistic web framework that uses C on the backend. This project will integrate with HTMX for the frontend and will use a custom markup language for the backend.

## Features

- [x] Custom markup language
- [x] HTMX integration
- [ ] Database integration

## Custom Markup Language

The custom markup language is a simple language that allows you to define routes and handlers in a simple way. It basically extends HTML with some custom tags.

### Example

```html
<?set name="title">
    My Awesome Web Page
</?set>

<?set name="user">
    <name>John Doe</name>
    <loggedIn>true</loggedIn>
</?set>

<html>
    <head>
        <title><?get name="title"></?get></title>
    </head>
    <body>
        <h1><?get name="title"></?get></h1>
        <p>Welcome, <?get name="user"><name></name></?get>!</p>

        <div hx-post="/update">
            <button>Click Me</button>
            <?hx-on>
                <?if>
                    <?get name="user"><loggedIn></loggedIn></?get>
                </?if>
                <?then>
                    <?return>
                        <p>Thank you for clicking the button, <?get name="user"><name></name></?get>!</p>
                    </?return>
                <?else>
                    <?return>
                        <p>Please log in to continue.</p>
                    </?return>
                <?/then>
            <?/hx-on>
        </div>
    </body>
</html>
```