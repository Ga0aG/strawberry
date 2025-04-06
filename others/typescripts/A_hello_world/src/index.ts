interface Greeting {
  text: string
  display: () => void
}

class HelloWorld implements Greeting {
  constructor(public text: string) {}

  display(): void {
    const el = document.getElementById('app')!
    el.innerHTML = `<h1>${this.text}</h1>`
  }
}

new HelloWorld("TypeScript!").display()